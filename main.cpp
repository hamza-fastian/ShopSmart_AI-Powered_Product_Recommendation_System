#include <iostream>
#include <string>
#include "Catalog.h"
#include "CustomerManager.h"
#include "Recommender.h"
#include "Scoring.h"
#include "FileHandler.h"
using namespace std;

int main() {
    Catalog catalog;
    CustomerManager mgr;

    cout << "Loading ShopSmart..." << endl;

    catalog.LoadProducts("data/products.csv");
    loadCustomers("data/customers.csv", mgr);

    int choice;

    do {
        cout << "\n========== ShopSmart ==========" << endl;
        cout << "1. Display All Products" << endl;
        cout << "2. Register Customer" << endl;
        cout << "3. Purchase Product" << endl;
        cout << "4. Get Recommendations" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            catalog.DisplayAllProducts();
        }

        else if (choice == 2) {
            string name;
            double budget;

            cout << "Enter customer name: ";
            cin.ignore();
            getline(cin, name);

            cout << "Enter budget: ";
            cin >> budget;

            int id = mgr.registerCustomer(name, budget);

            saveCustomers("data/customers.csv", mgr.getTable());

            cout << "Registration successful!" << endl;
            cout << "Your Customer ID: " << id << endl;
        }

        else if (choice == 3) {
            int customerId, productId;

            cout << "Enter Customer ID: ";
            cin >> customerId;

            Customer* customer = mgr.login(customerId);

            if (customer == nullptr) {
                cout << "Customer not found!" << endl;
                continue;
            }

            cout << "Enter Product ID: ";
            cin >> productId;

            Product* found = catalog.SearchProduct(productId);

            if (found == nullptr) {
                cout << "Product not found!" << endl;
                continue;
            }

            if (found->stock <= 0) {
                cout << "Product is out of stock!" << endl;
                continue;
            }

            if (found->price > customer->budget) {
                cout << "Insufficient budget!" << endl;
                continue;
            }

            Product p = *found;

            p.stock--;
            p.purchaseCount++;

            mgr.recordPurchase(customerId, productId);
            mgr.setBudget(customerId, customer->budget - p.price);

            catalog.UpdateProduct(p);

            saveProducts("data/products.csv", catalog.getAllProducts());
            saveCustomers("data/customers.csv", mgr.getTable());

            cout << "\nPurchase successful!" << endl;
            cout << "Product: " << p.name << endl;
            cout << "Remaining Stock: " << p.stock << endl;
            cout << "Remaining Budget: " << customer->budget << endl;
        }

        else if (choice == 4) {
            int customerId;

            cout << "Enter Customer ID: ";
            cin >> customerId;

            Customer* customer = mgr.login(customerId);

            if (customer == nullptr) {
                cout << "Customer not found!" << endl;
                continue;
            }

            vector<Product> top = getTopRecommendations(
                *customer,
                catalog.getAllProducts(),
                catalog.getProductTable(),
                5
            );

            cout << "\n===== Top Recommendations =====" << endl;

            if (top.empty()) {
                cout << "No recommendations available!" << endl;
            }

            for (int i = 0; i < top.size(); i++) {
                cout << "\n" << i + 1 << ". " << top[i].name << endl;
                cout << "Category: " << top[i].category << endl;
                cout << "Brand: " << top[i].brand << endl;
                cout << "Price: " << top[i].price << endl;
                cout << "Rating: " << top[i].rating << endl;

                cout << "Score: ";
                cout << scoreProduct(
                    *customer,
                    top[i],
                    catalog.getProductTable()
                ) << endl;
            }
        }

        else if (choice == 5) {
            saveProducts("data/products.csv", catalog.getAllProducts());
            saveCustomers("data/customers.csv", mgr.getTable());
            cout << "Thank you for using ShopSmart!" << endl;
        }

        else {
            cout << "Invalid choice!" << endl;
        }

    } while (choice != 5);

    return 0;
}