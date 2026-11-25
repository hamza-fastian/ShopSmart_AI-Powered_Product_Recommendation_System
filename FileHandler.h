#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Product.h"
#include "Customer.h"
#include "HashTable.h"
#include "CustomerManager.h"
using namespace std;

// Splits a line on a delimiter, keeping empty fields.
inline vector<string> splitLine(const string& s, char delim) {
    vector<string> parts;
    string cur;
    for (char ch : s) {
        if (ch == delim) { parts.push_back(cur); cur.clear(); }
        else cur += ch;
    }
    parts.push_back(cur);
    return parts;
}

// Splits a ';'-separated list field. An empty field gives an empty list.
inline vector<string> splitList(const string& s) {
    if (s.empty()) return {};
    return splitLine(s, ';');
}

// Reads products.csv. Header: id,name,category,brand,price,rating,purchaseCount
// Lines that cannot be parsed are skipped.
inline bool loadProducts(const string& filename, vector<Product>& out) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    getline(file, line);
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        vector<string> f = splitLine(line, ',');
        if (f.size() < 8) continue;
        try {
            Product p;
            p.id = stoi(f[0]);
            p.name = f[1];
            p.category = f[2];
            p.brand = f[3];
            p.price = stod(f[4]);
            p.rating = stod(f[5]);
            p.purchaseCount = stoi(f[6]);
            p.stock = stoi(f[7]);
            out.push_back(p);
        } catch (...) {
            continue;
        }
    }
    return true;
}

inline bool saveProducts(const string& filename, const vector<Product>& products) {
    ofstream file(filename);
    if (!file.is_open()) return false;

    file << "id,name,category,brand,price,rating,purchaseCount,stock\n";

    for (const Product& p : products) {
        file << p.id << "," << p.name << "," << p.category << ","
             << p.brand << "," << p.price << "," << p.rating << ","
             << p.purchaseCount << "," << p.stock << "\n";
    }

    return true;
}

// Writes customers.csv, one customer per line:
// id,name,budget,cat1;cat2,brand1;brand2,pid1;pid2
inline bool saveCustomers(const string& filename, HashTable<Customer>& table) {
    ofstream file(filename);
    if (!file.is_open()) return false;

    vector<Customer> all;
    table.getAll(all);
    for (const Customer& c : all) {
        file << c.id << "," << c.name << "," << c.budget << ",";
        for (size_t i = 0; i < c.preferredCategories.size(); i++) {
            if (i > 0) file << ";";
            file << c.preferredCategories[i];
        }
        file << ",";
        for (size_t i = 0; i < c.preferredBrands.size(); i++) {
            if (i > 0) file << ";";
            file << c.preferredBrands[i];
        }
        file << ",";
        for (size_t i = 0; i < c.purchaseHistory.size(); i++) {
            if (i > 0) file << ";";
            file << c.purchaseHistory[i];
        }
        file << "\n";
    }
    return true;
}

// Reads customers.csv back into the customer manager.
inline bool loadCustomers(const string& filename, CustomerManager& manager) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        vector<string> f = splitLine(line, ',');
        if (f.size() < 6) continue;
        try {
            Customer c;
            c.id = stoi(f[0]);
            c.name = f[1];
            c.budget = stod(f[2]);
            c.preferredCategories = splitList(f[3]);
            c.preferredBrands = splitList(f[4]);
            for (const string& s : splitList(f[5])) c.purchaseHistory.push_back(stoi(s));
            manager.addLoadedCustomer(c);
        } catch (...) {
            continue;
        }
    }
    return true;
}

#endif
