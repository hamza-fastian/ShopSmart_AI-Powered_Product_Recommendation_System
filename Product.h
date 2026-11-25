#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>
using namespace std;

// A single item in the store catalog.
struct Product {
    int id;
    string name;
    string category;
    string brand;
    double price;
    double rating;        // 0.0 to 5.0
    int purchaseCount;    // times bought, used as a popularity measure
    int stock;
};

#endif
