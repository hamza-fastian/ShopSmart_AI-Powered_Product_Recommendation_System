#ifndef BST_H
#define BST_H
#include<iostream>
#include "Product.h"
using namespace std;

struct Node {
    Product product;
    Node* left;
    Node* right;
};

class BST {
private:
    Node* root;

public:
    BST() {
        root = nullptr;
    }

    int Height(Node* p) {
        if(p == nullptr) {
            return 0;
        }

        int hl = Height(p->left);
        int hr = Height(p->right);

        return hl > hr ? hl + 1 : hr + 1;
    }

    Node* Insert(Product p) {
        Node* curr = root;
        Node* prev = nullptr;

        while(curr != nullptr) {
            prev = curr;

            if(p.price < curr->product.price) {
                curr = curr->left;
            }
            else if(p.price > curr->product.price) {
                curr = curr->right;
            }
            else {
                if(p.id == curr->product.id)
                    return curr;

                if(p.id < curr->product.id)
                    curr = curr->left;
                else
                    curr = curr->right;
            }
        }

        Node* newProd = new Node;
        newProd->product = p;
        newProd->left = nullptr;
        newProd->right = nullptr;

        if(prev == nullptr) {
            root = newProd;
        }
        else if(p.price < prev->product.price ||
                (p.price == prev->product.price && p.id < prev->product.id)) {
            prev->left = newProd;
        }
        else {
            prev->right = newProd;
        }

        return newProd;
    }

    Node* search(Node* r, double price, int id) {
        if(r == nullptr) {
            return nullptr;
        }

        if(r->product.price == price && r->product.id == id) {
            return r;
        }

        if(price < r->product.price ||
           (price == r->product.price && id < r->product.id)) {
            return search(r->left, price, id);
        }
        else {
            return search(r->right, price, id);
        }
    }

    void Inorder(Node* r) {
        if(r) {
            Inorder(r->left);

            cout << "Product Id: " << r->product.id << endl;
            cout << "Name: " << r->product.name << endl;
            cout << "Brand: " << r->product.brand << endl;
            cout << "Category: " << r->product.category << endl;
            cout << "Price: " << r->product.price << endl;
            cout << "Rating: " << r->product.rating << endl;
            cout << "Purchase Count: " << r->product.purchaseCount << endl;
            cout << "Stock: " << r->product.stock << endl;

            Inorder(r->right);
        }
    }

    Node* InSucc(Node* curr) {
        while(curr && curr->left != nullptr) {
            curr = curr->left;
        }
        return curr;
    }

    Node* InPre(Node* curr) {
        while(curr && curr->right != nullptr) {
            curr = curr->right;
        }
        return curr;
    }

    Node* Delete(Node* r, Product p) {
        if(r == nullptr) {
            return nullptr;
        }

        if(p.price < r->product.price ||
           (p.price == r->product.price && p.id < r->product.id)) {
            r->left = Delete(r->left, p);
        }
        else if(p.price > r->product.price ||
                (p.price == r->product.price && p.id > r->product.id)) {
            r->right = Delete(r->right, p);
        }
        else {
            if(r->left == nullptr && r->right == nullptr) {
                delete r;
                return nullptr;
            }

            if(r->left == nullptr) {
                Node* temp = r->right;
                delete r;
                return temp;
            }

            if(r->right == nullptr) {
                Node* temp = r->left;
                delete r;
                return temp;
            }

            Node* temp = InSucc(r->right);
            r->product = temp->product;
            r->right = Delete(r->right, temp->product);
        }

        return r;
    }

    void Delete(Product p) {
        root = Delete(root, p);
    }

    void RangeQuery(Node* r, double min, double max) {
        if(r == nullptr) {
            return;
        }

        if(r->product.price >= min) {
            RangeQuery(r->left, min, max);
        }

        if(r->product.price >= min && r->product.price <= max) {
            cout << "Product: " << r->product.name << endl;
            cout << "Price: " << r->product.price << endl;
        }

        if(r->product.price <= max) {
            RangeQuery(r->right, min, max);
        }
    }

    int GetHeight() {
        return Height(root);
    }

    Node* Search(double price, int id) {
        return search(root, price, id);
    }

    void Inorder() {
        Inorder(root);
    }

    void RangeQuery(double min, double max) {
        if(min > max) {
            cout << "Invalid price range!" << endl;
            return;
        }
        RangeQuery(root, min, max);
    }

    void destroy(Node* p) {
        if(p == nullptr) {
            return;
        }
        destroy(p->left);
        destroy(p->right);
        delete p;
    }

    ~BST() {
        destroy(root);
    }
};

#endif