#ifndef SCORING_H
#define SCORING_H

#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include "Product.h"
#include "Customer.h"
#include "HashTable.h"
using namespace std;

inline string toLower(string s) {
    for (char& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

// Case-insensitive check for whether a string is in a list.
inline bool listContains(const vector<string>& list, const string& item) {
    string target = toLower(item);
    for (const string& s : list) if (toLower(s) == target) return true;
    return false;
}

inline bool hasBought(const Customer& c, int productId) {
    for (int pid : c.purchaseHistory) if (pid == productId) return true;
    return false;
}

// Rule-based match score between a customer and a product:
//   +3  category is one of the customer's preferred categories
//   +2  brand is one of the customer's preferred brands
//   +2  price is within the customer's budget
//   +2  customer has bought from the same category before
//   +rating (0 to 5)
//   +1  product is popular (purchaseCount >= popularThreshold)
// The product hash table is used to look up the category of each past purchase.
// When explain is true, the points awarded are printed.
inline double scoreProduct(const Customer& c, const Product& p,
                           HashTable<Product>& productTable,
                           int popularThreshold = 50, bool explain = false) {
    double score = 0;

    if (listContains(c.preferredCategories, p.category)) {
        score += 3;
        if (explain) cout << "    +3 preferred category (" << p.category << ")\n";
    }
    if (listContains(c.preferredBrands, p.brand)) {
        score += 2;
        if (explain) cout << "    +2 preferred brand (" << p.brand << ")\n";
    }
    if (p.price <= c.budget) {
        score += 2;
        if (explain) cout << "    +2 within budget\n";
    }

    // Category bonus is applied once, even if several past purchases match.
    for (int pid : c.purchaseHistory) {
        Product* past = productTable.search(pid);
        if (past != nullptr && toLower(past->category) == toLower(p.category)) {
            score += 2;
            if (explain) cout << "    +2 bought from this category before\n";
            break;
        }
    }

    score += p.rating;
    if (explain) cout << "    +" << p.rating << " rating\n";

    if (p.purchaseCount >= popularThreshold) {
        score += 1;
        if (explain) cout << "    +1 popular\n";
    }
    return score;
}

#endif
