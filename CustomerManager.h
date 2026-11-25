#ifndef CUSTOMERMANAGER_H
#define CUSTOMERMANAGER_H

#include <iostream>
#include <string>
#include "Customer.h"
#include "HashTable.h"
using namespace std;

// Customer module. Customers are stored in a hash table keyed by customer ID.
class CustomerManager {
private:
    HashTable<Customer> customers;
    int nextId;

    bool contains(const vector<string>& list, const string& item) {
        for (const string& s : list) if (s == item) return true;
        return false;
    }

public:
    CustomerManager() : customers(11), nextId(1001) {}

    // Creates a customer with the next free ID and returns that ID.
    int registerCustomer(const string& name, double budget) {
        Customer c;
        c.id = nextId++;
        c.name = name;
        c.budget = budget;
        customers.insert(c.id, c);
        return c.id;
    }

    // Adds a customer read from a file and keeps nextId above every loaded ID.
    void addLoadedCustomer(const Customer& c) {
        customers.insert(c.id, c);
        if (c.id >= nextId) nextId = c.id + 1;
    }

    // Login is a hash table lookup by ID. Returns nullptr if the ID does not exist.
    Customer* login(int id) { return customers.search(id); }

    bool addPreferredCategory(int id, const string& category) {
        Customer* c = customers.search(id);
        if (c == nullptr) return false;
        if (!contains(c->preferredCategories, category)) c->preferredCategories.push_back(category);
        return true;
    }

    bool addPreferredBrand(int id, const string& brand) {
        Customer* c = customers.search(id);
        if (c == nullptr) return false;
        if (!contains(c->preferredBrands, brand)) c->preferredBrands.push_back(brand);
        return true;
    }

    bool setBudget(int id, double budget) {
        Customer* c = customers.search(id);
        if (c == nullptr) return false;
        c->budget = budget;
        return true;
    }

    // Adds a product ID to the customer's purchase history (no duplicates).
    bool recordPurchase(int customerId, int productId) {
        Customer* c = customers.search(customerId);
        if (c == nullptr) return false;
        for (int pid : c->purchaseHistory) if (pid == productId) return true;
        c->purchaseHistory.push_back(productId);
        return true;
    }

    void showCustomer(int id) {
        Customer* c = customers.search(id);
        if (c == nullptr) { cout << "Customer not found.\n"; return; }
        cout << "ID: " << c->id << " | Name: " << c->name << " | Budget: " << c->budget << "\n";
        cout << "  Categories:";
        for (const string& s : c->preferredCategories) cout << " " << s;
        cout << "\n  Brands:";
        for (const string& s : c->preferredBrands) cout << " " << s;
        cout << "\n  Purchased product IDs:";
        for (int pid : c->purchaseHistory) cout << " " << pid;
        cout << "\n";
    }

    HashTable<Customer>& getTable() { return customers; }
};

#endif
