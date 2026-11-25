#include <iostream>
#include "HashTable.h"
#include "MaxHeap.h"
#include "CustomerManager.h"
#include "Scoring.h"
#include "Recommender.h"
#include "FileHandler.h"
using namespace std;

int main() {
    // Hash table with int keys and values
    cout << "=== 1. Hash table (int -> int) ===\n";
    HashTable<int> ht(5);
    ht.insert(150, 1); ht.insert(101, 2); ht.insert(206, 3); ht.insert(304, 4);
    ht.displayStructure();
    cout << "search(206) found: " << (ht.search(206) != nullptr) << "  (expect 1)\n";
    cout << "search(999) found: " << (ht.search(999) != nullptr) << "  (expect 0)\n";
    ht.remove(206);
    cout << "after remove(206), found: " << (ht.search(206) != nullptr) << "  (expect 0)\n";
    for (int i = 1; i <= 20; i++) ht.insert(i, i);
    cout << "after 20 more inserts: buckets = " << ht.getSize()
         << ", count = " << ht.getCount() << ", load = " << ht.getLoadFactor() << "\n";

    // Heap: items should come out from largest to smallest
    cout << "\n=== 2. Max-heap ===\n";
    MaxHeap heap;
    double scores[] = {5.5, 9.0, 2.0, 7.5, 9.5, 1.0, 6.0};
    for (int i = 0; i < 7; i++) heap.insert({i, scores[i]});
    cout << "Popping in order: ";
    ScoredItem it;
    while (heap.extractMax(it)) cout << it.score << " ";
    cout << "\n(expect descending: 9.5 9 7.5 6 5.5 2 1)\n";

    // Load the product catalog and index it by product ID
    cout << "\n=== 3. Load products from CSV ===\n";
    vector<Product> products;
    if (!loadProducts("data/products.csv", products)) {
        cout << "Could not open data/products.csv\n";
        return 1;
    }
    HashTable<Product> productTable(11);
    for (const Product& p : products) productTable.insert(p.id, p);
    cout << "Loaded " << products.size() << " products into the product hash table.\n";
    Product* found = productTable.search(10);
    if (found) cout << "Lookup product 10: " << found->name << "\n";

    // Register a customer with preferences and one past purchase
    cout << "\n=== 4. Customer module ===\n";
    CustomerManager mgr;
    int id = mgr.registerCustomer("Ayesha", 10000);
    mgr.addPreferredCategory(id, "Electronics");
    mgr.addPreferredCategory(id, "Books");
    mgr.addPreferredBrand(id, "Sony");
    mgr.recordPurchase(id, 9);
    mgr.showCustomer(id);

    // Top 5 recommendations with the score breakdown for each
    cout << "\n=== 5. Top 5 recommendations ===\n";
    Customer* c = mgr.login(id);
    vector<Product> top = getTopRecommendations(*c, products, productTable, 5);
    for (size_t i = 0; i < top.size(); i++) {
        cout << (i + 1) << ". " << top[i].name << " (" << top[i].category
             << ", " << top[i].brand << ", Rs " << top[i].price << ")\n";
        scoreProduct(*c, top[i], productTable, 50, true);
    }

    // Save customers to a file and load them into a fresh manager
    cout << "\n=== 6. Save and reload customers ===\n";
    saveCustomers("data/customers.csv", mgr.getTable());
    CustomerManager mgr2;
    loadCustomers("data/customers.csv", mgr2);
    mgr2.showCustomer(id);
    int id2 = mgr2.registerCustomer("Bilal", 5000);
    cout << "Next new customer got ID " << id2 << " (expect " << id + 1 << ")\n";

    return 0;
}
