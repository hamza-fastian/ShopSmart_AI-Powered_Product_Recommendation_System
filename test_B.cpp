
#include<iostream>
#include "Catalog.h"
#include "CustomerManager.h"
#include "Recommender.h"
#include "Scoring.h"
using namespace std;

int main() {
    Catalog catalog;

    // Load products from CSV
    cout << "=== 1. Load Products ===" << endl;
    catalog.LoadProducts("data/products.csv");

    // Display products using Circular Linked List
    cout << "\n=== 2. All Products (Circular Linked List) ===" << endl;
    catalog.DisplayAllProducts();

    // Display products according to price using BST
    cout << "\n=== 3. Products By Price (BST) ===" << endl;
    catalog.DisplayByPrice();

    // Display products according to name using AVL
    cout << "\n=== 4. Products By Name (AVL) ===" << endl;
    catalog.DisplayByName();

    // Search product using Hash Table
    cout << "\n=== 5. Product Search (Hash Table) ===" << endl;

    Product* found = catalog.SearchProduct(10);

    if(found != nullptr) {
        cout << "Product Found: " << found->name << endl;
        cout << "Price: " << found->price << endl;
        cout << "Stock: " << found->stock << endl;
    }
    else {
        cout << "Product not found!" << endl;
    }

    // Range query using BST
    cout << "\n=== 6. Price Range Query ===" << endl;
    catalog.RangeQuery(100, 10000);

    // Register customer
    cout << "\n=== 7. Customer Management ===" << endl;

    CustomerManager mgr;

    int id = mgr.registerCustomer("Ayesha", 10000);

    mgr.addPreferredCategory(id, "Electronics");
    mgr.addPreferredCategory(id, "Books");
    mgr.addPreferredBrand(id, "Sony");

    // Purchase product
    cout << "\n=== 8. Product Purchase ===" << endl;

    found = catalog.SearchProduct(9);

    Customer* customer = mgr.login(id);

    if(found != nullptr && customer != nullptr) {
        if(found->stock > 0 && found->price <= customer->budget) {
            Product p = *found;

            p.stock--;
            p.purchaseCount++;

            mgr.recordPurchase(id, 9);
            mgr.setBudget(id, customer->budget - p.price);

            catalog.UpdateProduct(p);

            cout << p.name << " purchased successfully!" << endl;
            cout << "Remaining Stock: " << p.stock << endl;
            cout << "Remaining Budget: " << customer->budget << endl;
        }
        else if(found->stock <= 0) {
            cout << "Product is out of stock!" << endl;
        }
        else {
            cout << "Insufficient budget!" << endl;
        }
    }
    else {
        cout << "Product or customer not found!" << endl;
    }

    mgr.showCustomer(id);

    // Generate recommendations using Max Heap
    cout << "\n=== 9. Top 5 Recommendations ===" << endl;

    Customer* c = mgr.login(id);

    if(c != nullptr) {
        vector<Product> top = getTopRecommendations(
            *c,
            catalog.getAllProducts(),
            catalog.getProductTable(),
            5
        );

        for(int i = 0; i < top.size(); i++) {
            cout << "\n" << i + 1 << ". "
                 << top[i].name << endl;

            cout << "Category: " << top[i].category << endl;
            cout << "Brand: " << top[i].brand << endl;
            cout << "Price: " << top[i].price << endl;
            cout << "Stock: " << top[i].stock << endl;

            cout << "Score Breakdown:" << endl;

            scoreProduct(*c, top[i], catalog.getProductTable(), 50, true);
        }
    }

    // Circular Linked List traversal
    cout << "\n=== 10. Circular Navigation ===" << endl;

    cout << "\nNext Product:\n";
    catalog.NextProduct();

    cout << "\nPrevious Product:\n";
    catalog.PreviousProduct();

    return 0;
}