# ShopSmart Frontend

A light, modern web UI for the ShopSmart C++ recommendation system.

It talks to your **real** backend through a small C++ API server (`server.cpp`, in the repository root). That server `#include`s your existing headers (`Catalog.h`, `CustomerManager.h`, `Recommender.h`, `Scoring.h`, `FileHandler.h`...) and calls them unchanged, so products, logins, scores and rankings come from your Hash Table, Max Heap and `scoreProduct()` code, not from JavaScript.

```
Browser (frontend/)  ──HTTP/JSON──▶  server.cpp  ──▶  Catalog · CustomerManager · Recommender · Scoring
                                                      (Hash Table, BST, AVL, Circular List, Max Heap)
                                                      data/products.csv · data/customers.csv
```

## 1. Requirements

- **A C++ compiler with C++17** (GCC / MinGW-w64 8 or newer).
  - **Windows:** install MSYS2 or WinLibs (MinGW-w64), then check that `g++ --version` works in the VS Code terminal.
  - **Linux:** `sudo apt install g++`. **macOS:** `xcode-select --install`.
- A modern browser.
- **No Node.js, no npm, no internet needed.** The UI is plain HTML/CSS/JS and the Inter font is bundled in `frontend/assets/fonts/`. `third_party/httplib.h` (cpp-httplib, MIT) is already in the repository.

## 2. Compile the server

Open the repository **root** folder in VS Code (the one containing `main.cpp`, `server.cpp`, `data/`, `frontend/`) and run in the VS Code terminal:

**Windows (MinGW g++):**
```bash
g++ -std=c++17 server.cpp -o server.exe -lws2_32 -lmswsock
```

**Linux / macOS:**
```bash
g++ -std=c++17 server.cpp -o server -pthread
```

The first compile takes about 1–3 minutes because `httplib.h` is a large header. This is normal.

## 3. Start the server

Still in the repository root:

```bash
.\server.exe        # Windows PowerShell   (in cmd.exe: server.exe)
./server            # Linux / macOS
```

You should see:

```
Products loaded successfully!
Product photos found: 0/100 (products without a photo show an icon tile)
ShopSmart running at http://localhost:8080  (Ctrl+C to stop)
```

If Windows Firewall asks, allow access for private networks.

## 4. Open the frontend

Go to **http://localhost:8080** in your browser. The server serves both the website and the API on port **8080**, so there is nothing else to start and no CORS setup.

To use another port: `.\server.exe 9000`, then open `http://localhost:9000`.

## 5. How the frontend talks to the C++ backend

Every page loads `js/api.js`, the only place that knows the backend address. It first asks the page's own origin for `/api/health` (this succeeds when the page is served by `server.cpp`); otherwise it falls back to `BACKEND_URL` at the top of `api.js` (default `http://localhost:8080`). You can also force a server once with `?api=http://localhost:9000`. The server sends CORS headers, so opening the HTML via VS Code Live Server also works while `server.exe` is running.

| Method | Path | Backend code used |
|---|---|---|
| GET | `/api/health` | – |
| GET | `/api/products`, `/api/products/:id` | `Catalog`, hash-table `SearchProduct` |
| GET | `/api/product-images` | lists photos present in `assets/images/products/` |
| POST | `/api/register` `{name,budget,preferredCategories?}` | `CustomerManager::registerCustomer` + `saveCustomers` |
| POST | `/api/login` `{id}` | `CustomerManager::login` (hash-table lookup) |
| GET | `/api/customers/:id` | `CustomerManager::login` |
| POST | `/api/customers/:id/preferences` `{budget?,preferredCategories?,preferredBrands?}` | `setBudget`, `addPreferredCategory/Brand` + `saveCustomers` |
| POST | `/api/customers/:id/purchase` `{productId}` | same rules and order of checks as option 3 in `main.cpp`; saves both CSVs |
| GET | `/api/recommendations/:id?k=10` | `getTopRecommendations` (Max Heap) + `scoreProduct(explain=true)` for the point breakdown |

The "why was this recommended" breakdown is captured from your own `scoreProduct(..., explain=true)` output, so it can never disagree with the real scoring.

## Pages

| Page | What it does |
|---|---|
| `index.html` | Landing page: hero, categories, top-rated products, recommendation-engine explainer |
| `signup.html` / `login.html` | Register / log in |
| `dashboard.html` | Welcome, stats, top picks, recent purchases, search |
| `products.html` | Search, category filter, sort, product details modal, buy |
| `recommendations.html` | Full ranked list with match score and the points behind each |
| `preferences.html` | Edit budget, preferred categories and brands; live top-3 preview from the real recommender |
| `history.html` | Purchased products |
| `profile.html` | Account details, log out |

## Logging in

The original backend identifies customers by **ID** (`mgr.login(id)`); it has no passwords, so none are faked. **Sign up** shows your new Customer ID. **Log in** takes that ID. Customers already in `data/customers.csv` (for example `1001`) work too.

## Product photos

`data/products.csv` has no images, and real photographs could not be downloaded while this was built (the build sandbox blocked image sites). So the UI ships with a clean icon tile per product type, and photos are a drop-in:

1. Save a photo as `frontend/assets/images/products/<productId>.jpg` (`.jpeg`, `.png`, `.webp` also work), for example `5.jpg` for the Laptop.
2. Refresh the page. No code changes. The server prints `Product photos found: N/100` at startup.

`frontend/assets/images/products/README.md` lists all 100 products with the exact filename each one needs. Use photos you own or that are licensed for reuse (Wikimedia Commons, Unsplash, Pexels).

## Data is shared with `main.cpp`

The server reads and writes the same `data/*.csv` files as the console app, exactly as `main.cpp` does, so registrations and purchases appear in both. Don't run `server` and the console app at the same time, because each loads the CSVs at startup.

## Known limitations (from the existing backend)

- No passwords: login is by customer ID.
- Purchase history stores product IDs only (no dates or quantities), so none are shown.
- A customer's name cannot be changed (no such function exists), so Profile is read-only for the name.
- "Budget" doubles as the remaining balance: purchases reduce it.

## Troubleshooting

| Problem | Fix |
|---|---|
| "Cannot reach the ShopSmart server" in the UI | The server isn't running. Start it from the repo root (step 3). |
| `g++` is not recognized | MinGW is not installed or not on PATH. Install MSYS2/WinLibs and reopen VS Code. |
| `No products loaded` at startup | Run from the repo root, or pass the data folder: `.\server.exe 8080 path\to\data` |
| `Could not start on port 8080` | Another program uses the port. Use `.\server.exe 9000`. |
| Windows linker errors (`WSAStartup`, `socket`...) | Make sure the compile command includes `-lws2_32 -lmswsock`. |
| Opened `index.html` by double-clicking | Use `http://localhost:8080` so the server is reached. |
| Changed `data/*.csv` by hand | Restart the server. It loads the CSVs once at startup. |
