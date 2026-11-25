#ifndef RECOMMENDER_H
#define RECOMMENDER_H

#include <vector>
#include "Product.h"
#include "Customer.h"
#include "HashTable.h"
#include "MaxHeap.h"
#include "Scoring.h"
using namespace std;

// Scores every product the customer has not bought, pushes the scores into a
// max-heap, and returns the K highest-scoring products in order.
inline vector<Product> getTopRecommendations(const Customer& c,
                                             const vector<Product>& allProducts,
                                             HashTable<Product>& productTable,
                                             int k) {
    MaxHeap heap;
    if (k <= 0) return {};
    for (const Product& p : allProducts) {
        if (p.stock <= 0) continue;
        if (hasBought(c, p.id)) continue;
        heap.insert({p.id, scoreProduct(c, p, productTable)});
    }

    vector<Product> result;
    ScoredItem item;
    while (result.size() < (size_t)k && heap.extractMax(item)) {
        Product* p = productTable.search(item.productId);
        if (p != nullptr) result.push_back(*p);
    }
    return result;
}

#endif
