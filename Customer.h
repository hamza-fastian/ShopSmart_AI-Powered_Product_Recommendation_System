#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>
#include <vector>
using namespace std;

// A registered customer with their preferences and purchase history.
struct Customer {
    int id;
    string name;
    double budget;                       // maximum price the customer likes to pay
    vector<string> preferredCategories;
    vector<string> preferredBrands;
    vector<int> purchaseHistory;         // IDs of products already bought
};

#endif
