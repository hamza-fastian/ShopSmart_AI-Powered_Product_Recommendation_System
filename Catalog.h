#ifndef CATALOG_H
#define CATALOG_H
#include "Product.h"
#include "BST.h"
#include "AVL.h"
#include "CircularLL.h"
#include "HashTable.h"
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
using namespace std;

class Catalog {
private:
    BST bst;
    AVL avl;
    CircularLL cll;

    HashTable<Product> productTable;
    vector<Product> products;

public:
    Catalog() : productTable(11) {}

    void AddProduct(Product p) {
        if(productTable.search(p.id) != nullptr) {
            return;
        }

        productTable.insert(p.id, p);
        products.push_back(p);

        bst.Insert(p);
        avl.Insert(p);
        cll.AddProduct(p);
    }

    void RemoveProduct(Product p) {
        Product* found = productTable.search(p.id);

        if(found == nullptr) {
            return;
        }

        Product oldProduct = *found;

        bst.Delete(oldProduct);
        avl.Delete(oldProduct.name, oldProduct.id);
        cll.RemoveProduct(oldProduct.id);

        productTable.remove(oldProduct.id);

        for(int i = 0; i < products.size(); i++) {
            if(products[i].id == oldProduct.id) {
                products.erase(products.begin() + i);
                break;
            }
        }
    }

    void RemoveProduct(int id) {
        Product* found = productTable.search(id);

        if(found == nullptr) {
            return;
        }

        Product p = *found;
        RemoveProduct(p);
    }

    void UpdateProduct(Product p) {
        if(productTable.search(p.id) == nullptr) {
            return;
        }
        RemoveProduct(p);
        AddProduct(p);
    }

    void DisplayByPrice() {
        bst.Inorder();
    }

    void DisplayByName() {
        avl.Inorder();
    }

    void DisplayAllProducts() {
        cll.Display();
    }

    void RangeQuery(double min, double max) {
        bst.RangeQuery(min, max);
    }

    Product* SearchProduct(int id) {
        return productTable.search(id);
    }

    vector<Product>& getAllProducts() {
        return products;
    }

    HashTable<Product>& getProductTable() {
        return productTable;
    }

    void NextProduct() {
        cll.NextProduct();
    }

    void PreviousProduct() {
        cll.PreviousProduct();
    }

    void LoadProducts(string filename) {
        ifstream file(filename);

        if(!file.is_open()) {
            cout << "File could not be opened!" << endl;
            return;
        }

        string line;
        getline(file, line);

        while(getline(file, line)) {
            if(!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if(line.empty()) {
                continue;
            }

            stringstream ss(line);

            string id, name, category, brand;
            string price, rating, purchaseCount, stock;

            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, category, ',');
            getline(ss, brand, ',');
            getline(ss, price, ',');
            getline(ss, rating, ',');
            getline(ss, purchaseCount, ',');
            getline(ss, stock, ',');

            try {
                Product p;

                p.id = stoi(id);
                p.name = name;
                p.category = category;
                p.brand = brand;
                p.price = stod(price);
                p.rating = stod(rating);
                p.purchaseCount = stoi(purchaseCount);
                p.stock = stoi(stock);

                AddProduct(p);
            }
            catch(...) {
                continue;
            }
        }

        file.close();

        cout << "Products loaded successfully!" << endl;
    }
};

#endif