// ShopSmart HTTP API
// ------------------------------------------------------------------
// A thin integration layer that lets the browser frontend talk to the
// existing ShopSmart C++ backend. It does NOT re-implement anything: it
// #includes the original headers and calls Catalog, CustomerManager,
// getTopRecommendations() and scoreProduct() exactly as main.cpp does.
//
//   g++ -std=c++17 server.cpp -o server -pthread          (Linux / macOS)
//   g++ -std=c++17 server.cpp -o server -lws2_32 -lmswsock (Windows MinGW)
//
// Run from the repository root:   ./server   (then open http://localhost:8080)
// Optional arguments:             ./server [port] [dataDir] [frontendDir]

// httplib.h MUST come first: on Windows it pulls in <windows.h>, whose `byte` typedef clashes with
// std::byte if it is included after the project headers (they contain `using namespace std;`).
#include "third_party/httplib.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "Catalog.h"
#include "CustomerManager.h"
#include "FileHandler.h"
#include "Recommender.h"
#include "Scoring.h"

using namespace std;

// ---------- globals (the same objects main.cpp creates) ----------
static Catalog catalog;
static CustomerManager mgr;
static mutex backendLock;  // the data structures are not thread-safe
static string productsFile = "data/products.csv";
static string customersFile = "data/customers.csv";

// ---------- tiny JSON helpers ----------
static string esc(const string& s) {
    string o;
    for (unsigned char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (c < 0x20) { char b[8]; snprintf(b, sizeof b, "\\u%04x", c); o += b; }
                else o += (char)c;
        }
    }
    return o;
}
static string q(const string& s) { return "\"" + esc(s) + "\""; }
static string num(double d) {
    ostringstream os;
    os << d;
    return os.str();
}

static string productJson(const Product& p) {
    return "{\"id\":" + to_string(p.id) + ",\"name\":" + q(p.name) +
           ",\"category\":" + q(p.category) + ",\"brand\":" + q(p.brand) +
           ",\"price\":" + num(p.price) + ",\"rating\":" + num(p.rating) +
           ",\"purchaseCount\":" + to_string(p.purchaseCount) +
           ",\"stock\":" + to_string(p.stock) + "}";
}

static string strListJson(const vector<string>& v) {
    string o = "[";
    for (size_t i = 0; i < v.size(); i++) { if (i) o += ","; o += q(v[i]); }
    return o + "]";
}

static string customerJson(const Customer& c) {
    string o = "{\"id\":" + to_string(c.id) + ",\"name\":" + q(c.name) +
               ",\"budget\":" + num(c.budget) +
               ",\"preferredCategories\":" + strListJson(c.preferredCategories) +
               ",\"preferredBrands\":" + strListJson(c.preferredBrands) +
               ",\"purchaseHistory\":[";
    for (size_t i = 0; i < c.purchaseHistory.size(); i++) {
        if (i) o += ",";
        o += to_string(c.purchaseHistory[i]);
    }
    return o + "]}";
}

// Tiny flat-JSON reader for request bodies such as {"name":"Ali","budget":5000}
static bool jsonString(const string& body, const string& key, string& out) {
    size_t k = body.find("\"" + key + "\"");
    if (k == string::npos) return false;
    size_t colon = body.find(':', k);
    if (colon == string::npos) return false;
    size_t a = body.find('"', colon);
    if (a == string::npos) return false;
    string v;
    for (size_t i = a + 1; i < body.size(); i++) {
        if (body[i] == '\\' && i + 1 < body.size()) { v += body[++i]; continue; }
        if (body[i] == '"') { out = v; return true; }
        v += body[i];
    }
    return false;
}
static bool jsonNumber(const string& body, const string& key, double& out) {
    size_t k = body.find("\"" + key + "\"");
    if (k == string::npos) return false;
    size_t colon = body.find(':', k);
    if (colon == string::npos) return false;
    size_t i = colon + 1;
    while (i < body.size() && (body[i] == ' ' || body[i] == '"')) i++;
    size_t j = i;
    while (j < body.size() && (isdigit((unsigned char)body[j]) || body[j] == '.' || body[j] == '-')) j++;
    if (j == i) return false;
    try { out = stod(body.substr(i, j - i)); return true; } catch (...) { return false; }
}
static vector<string> jsonStringArray(const string& body, const string& key) {
    vector<string> out;
    size_t k = body.find("\"" + key + "\"");
    if (k == string::npos) return out;
    size_t a = body.find('[', k), b = body.find(']', k);
    if (a == string::npos || b == string::npos || b < a) return out;
    size_t i = a;
    while (true) {
        size_t s = body.find('"', i);
        if (s == string::npos || s > b) break;
        size_t e = body.find('"', s + 1);
        if (e == string::npos || e > b) break;
        out.push_back(body.substr(s + 1, e - s - 1));
        i = e + 1;
    }
    return out;
}

static void reply(httplib::Response& res, int status, const string& json) {
    res.status = status;
    res.set_content(json, "application/json");
}
static void fail(httplib::Response& res, int status, const string& msg) {
    reply(res, status, "{\"error\":" + q(msg) + "}");
}

// Runs the project's own scoreProduct(explain=true) and captures what it prints,
// so the "why was this recommended" breakdown comes from the real scoring code.
static string explainJson(const Customer& c, const Product& p) {
    ostringstream captured;
    streambuf* old = cout.rdbuf(captured.rdbuf());
    double score = scoreProduct(c, p, catalog.getProductTable(), 50, true);
    cout.rdbuf(old);

    string reasons = "[";
    istringstream lines(captured.str());
    string line;
    bool first = true;
    while (getline(lines, line)) {
        size_t a = line.find('+');
        if (a == string::npos) continue;
        size_t sp = line.find(' ', a);
        double pts = 0;
        try { pts = stod(line.substr(a + 1, sp - a - 1)); } catch (...) {}
        string label = sp == string::npos ? "" : line.substr(sp + 1);
        if (!first) reasons += ",";
        first = false;
        reasons += "{\"points\":" + num(pts) + ",\"reason\":" + q(label) + "}";
    }
    reasons += "]";
    return "{\"score\":" + num(score) + ",\"breakdown\":" + reasons + "}";
}

int main(int argc, char** argv) {
    int port = 8080;
    string dataDir = "data";
    string frontendDir = "frontend";
    if (argc > 1) port = atoi(argv[1]);
    if (argc > 2) dataDir = argv[2];
    if (argc > 3) frontendDir = argv[3];
    productsFile = dataDir + "/products.csv";
    customersFile = dataDir + "/customers.csv";

    cout << "Loading ShopSmart data from '" << dataDir << "'..." << endl;
    catalog.LoadProducts(productsFile);
    loadCustomers(customersFile, mgr);
    if (catalog.getAllProducts().empty()) {
        cerr << "No products loaded. Run the server from the repository root "
                "(or pass the data folder as the 2nd argument)." << endl;
        return 1;
    }

    {   // report which products already have a photo (frontend/assets/images/products/<id>.jpg|png|webp)
        int have = 0;
        for (const Product& p : catalog.getAllProducts())
            for (const char* e : {"jpg", "jpeg", "png", "webp"})
                if (ifstream(frontendDir + "/assets/images/products/" + to_string(p.id) + "." + e).good()) { have++; break; }
        cout << "Product photos found: " << have << "/" << catalog.getAllProducts().size()
             << " (products without a photo show an icon tile)" << endl;
    }

    httplib::Server svr;

    // CORS: lets the UI also work when opened from another origin (e.g. VS Code Live Server).
    svr.set_default_headers({{"Access-Control-Allow-Origin", "*"},
                             {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
                             {"Access-Control-Allow-Headers", "Content-Type"}});
    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) { res.status = 204; });

    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        reply(res, 200, "{\"status\":\"ok\",\"products\":" + to_string(catalog.getAllProducts().size()) + "}");
    });

    // Which product photos exist? Drop files named <productId>.jpg/.jpeg/.png/.webp into
    // frontend/assets/images/products/ and they are picked up automatically (no code edits).
    svr.Get("/api/product-images", [frontendDir](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        const char* exts[] = {"jpg", "jpeg", "png", "webp"};
        string o = "{";
        bool first = true;
        for (const Product& p : catalog.getAllProducts()) {
            for (const char* e : exts) {
                string file = to_string(p.id) + "." + e;
                ifstream f(frontendDir + "/assets/images/products/" + file, ios::binary);
                if (f.good()) {
                    if (!first) o += ",";
                    first = false;
                    o += "\"" + to_string(p.id) + "\":" + q(file);
                    break;
                }
            }
        }
        reply(res, 200, o + "}");
    });

    // ---- products ----
    svr.Get("/api/products",[](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        string o = "[";
        bool first = true;
        for (const Product& p : catalog.getAllProducts()) {
            if (!first) o += ",";
            first = false;
            o += productJson(p);
        }
        reply(res, 200, o + "]");
    });

    // Hash-table lookup by ID
    svr.Get(R"(/api/products/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        Product* p = catalog.SearchProduct(stoi(req.matches[1]));
        if (!p) return fail(res, 404, "Product not found");
        reply(res, 200, productJson(*p));
    });

    // ---- customers ----
    svr.Post("/api/register", [](const httplib::Request& req, httplib::Response& res) {
        string name; double budget;
        if (!jsonString(req.body, "name", name) || name.empty()) return fail(res, 400, "Name is required");
        if (!jsonNumber(req.body, "budget", budget) || budget < 0) return fail(res, 400, "A valid budget is required");
        if (name.find(',') != string::npos || name.find('\n') != string::npos)
            return fail(res, 400, "Name cannot contain commas");
        if (name.size() > 60) return fail(res, 400, "Name is too long");

        lock_guard<mutex> g(backendLock);
        int id = mgr.registerCustomer(name, budget);
        for (const string& c : jsonStringArray(req.body, "preferredCategories"))
            if (c.find(',') == string::npos && c.find(';') == string::npos) mgr.addPreferredCategory(id, c);
        for (const string& b : jsonStringArray(req.body, "preferredBrands"))
            if (b.find(',') == string::npos && b.find(';') == string::npos) mgr.addPreferredBrand(id, b);
        saveCustomers(customersFile, mgr.getTable());
        reply(res, 201, customerJson(*mgr.login(id)));
    });

    // The backend's "login" is a hash-table lookup by customer ID (there are no passwords).
    svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
        double id;
        if (!jsonNumber(req.body, "id", id)) return fail(res, 400, "Customer ID is required");
        lock_guard<mutex> g(backendLock);
        Customer* c = mgr.login((int)id);
        if (!c) return fail(res, 404, "Customer not found");
        reply(res, 200, customerJson(*c));
    });

    svr.Get(R"(/api/customers/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        Customer* c = mgr.login(stoi(req.matches[1]));
        if (!c) return fail(res, 404, "Customer not found");
        reply(res, 200, customerJson(*c));
    });

    // Update budget / preferred categories / brands (CustomerManager already supports this).
    svr.Post(R"(/api/customers/(\d+)/preferences)", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        int id = stoi(req.matches[1]);
        Customer* c = mgr.login(id);
        if (!c) return fail(res, 404, "Customer not found");
        double budget;
        if (jsonNumber(req.body, "budget", budget) && budget >= 0) mgr.setBudget(id, budget);
        // ',' and ';' are the CSV separators in customers.csv, so such values are skipped.
        auto safe = [](const string& s) {
            return !s.empty() && s.size() <= 40 && s.find_first_of(",;\n\r\"\\") == string::npos;
        };
        if (req.body.find("\"preferredCategories\"") != string::npos) {
            c->preferredCategories.clear();
            for (const string& s : jsonStringArray(req.body, "preferredCategories"))
                if (safe(s)) mgr.addPreferredCategory(id, s);
        }
        if (req.body.find("\"preferredBrands\"") != string::npos) {
            c->preferredBrands.clear();
            for (const string& s : jsonStringArray(req.body, "preferredBrands"))
                if (safe(s)) mgr.addPreferredBrand(id, s);
        }
        saveCustomers(customersFile, mgr.getTable());
        reply(res, 200, customerJson(*c));
    });

    // Same rules and order of checks as option 3 in main.cpp.
    svr.Post(R"(/api/customers/(\d+)/purchase)", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        int cid = stoi(req.matches[1]);
        double pidD;
        if (!jsonNumber(req.body, "productId", pidD)) return fail(res, 400, "productId is required");
        Customer* customer = mgr.login(cid);
        if (!customer) return fail(res, 404, "Customer not found");
        Product* found = catalog.SearchProduct((int)pidD);
        if (!found) return fail(res, 404, "Product not found");
        if (found->stock <= 0) return fail(res, 409, "Product is out of stock");
        if (found->price > customer->budget) return fail(res, 409, "Insufficient budget");

        Product p = *found;
        p.stock--;
        p.purchaseCount++;
        mgr.recordPurchase(cid, p.id);
        mgr.setBudget(cid, customer->budget - p.price);
        catalog.UpdateProduct(p);
        saveProducts(productsFile, catalog.getAllProducts());
        saveCustomers(customersFile, mgr.getTable());
        reply(res, 200, "{\"product\":" + productJson(p) + ",\"customer\":" + customerJson(*customer) + "}");
    });

    // ---- recommendations: heap-ranked, scored by the project's own scoreProduct() ----
    svr.Get(R"(/api/recommendations/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> g(backendLock);
        Customer* c = mgr.login(stoi(req.matches[1]));
        if (!c) return fail(res, 404, "Customer not found");
        int k = 5;
        if (req.has_param("k")) k = atoi(req.get_param_value("k").c_str());
        if (k < 1) k = 1;
        if (k > 200) k = 200;

        vector<Product> top = getTopRecommendations(*c, catalog.getAllProducts(), catalog.getProductTable(), k);
        string o = "[";
        for (size_t i = 0; i < top.size(); i++) {
            if (i) o += ",";
            string ex = explainJson(*c, top[i]);  // {"score":..,"breakdown":[..]}
            o += "{\"rank\":" + to_string(i + 1) + ",\"product\":" + productJson(top[i]) + "," + ex.substr(1);
        }
        reply(res, 200, o + "]");
    });

    // ---- static frontend (same origin as the API, so no CORS needed) ----
    if (!svr.set_mount_point("/", frontendDir)) {
        cerr << "Warning: frontend folder '" << frontendDir << "' not found; API only." << endl;
    }

    cout << "ShopSmart running at http://localhost:" << port << "  (Ctrl+C to stop)" << endl;
    if (!svr.listen("0.0.0.0", port)) {
        cerr << "Could not start on port " << port << " (is it already in use?)" << endl;
        return 1;
    }
    return 0;
}
