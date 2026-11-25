#ifndef AVL_H
#define AVL_H
#include<iostream>
#include "Product.h"
using namespace std;

struct NodeAvl {
    Product prod;
    int height;
    NodeAvl* left;
    NodeAvl* right;
};

class AVL {
private:
    NodeAvl* root;

public:
    AVL() {
        root = nullptr;
    }

    int Height(NodeAvl* p) {
        if(p == nullptr) {
            return 0;
        }

        int hl = (p && p->left) ? p->left->height : 0;
        int hr = (p && p->right) ? p->right->height : 0;

        return hl > hr ? hl + 1 : hr + 1;
    }

    int BalanceFactor(NodeAvl* p) {
        int hl = (p && p->left) ? p->left->height : 0;
        int hr = (p && p->right) ? p->right->height : 0;

        return hl - hr;
    }

    NodeAvl* LLRotation(NodeAvl* p) {
        NodeAvl* pl = p->left;
        NodeAvl* plr = pl->right;

        pl->right = p;
        p->left = plr;

        p->height = Height(p);
        pl->height = Height(pl);

        return pl;
    }

    NodeAvl* RRRotation(NodeAvl* p) {
        NodeAvl* pr = p->right;
        NodeAvl* prl = pr->left;

        pr->left = p;
        p->right = prl;

        p->height = Height(p);
        pr->height = Height(pr);

        return pr;
    }

    NodeAvl* LRRotation(NodeAvl* p) {
        p->left = RRRotation(p->left);
        return LLRotation(p);
    }

    NodeAvl* RLRotation(NodeAvl* p) {
        p->right = LLRotation(p->right);
        return RRRotation(p);
    }

    NodeAvl* Insertion(NodeAvl* p, Product pr) {
        if(p == nullptr) {
            NodeAvl* newProd = new NodeAvl;
            newProd->prod = pr;
            newProd->height = 1;
            newProd->left = nullptr;
            newProd->right = nullptr;
            return newProd;
        }

        if(pr.name < p->prod.name ||
           (pr.name == p->prod.name && pr.id < p->prod.id)) {
            p->left = Insertion(p->left, pr);
        }
        else if(pr.name > p->prod.name ||
                (pr.name == p->prod.name && pr.id > p->prod.id)) {
            p->right = Insertion(p->right, pr);
        }
        else {
            return p;
        }

        p->height = Height(p);

        if(BalanceFactor(p) == 2 && BalanceFactor(p->left) == 1) {
            return LLRotation(p);
        }
        else if(BalanceFactor(p) == 2 && BalanceFactor(p->left) == -1) {
            return LRRotation(p);
        }
        else if(BalanceFactor(p) == -2 && BalanceFactor(p->right) == -1) {
            return RRRotation(p);
        }
        else if(BalanceFactor(p) == -2 && BalanceFactor(p->right) == 1) {
            return RLRotation(p);
        }

        return p;
    }

    NodeAvl* Search(NodeAvl* p, string name, int id) {
        while(p != nullptr) {
            if(name == p->prod.name && id == p->prod.id) {
                return p;
            }
            else if(name < p->prod.name ||
                    (name == p->prod.name && id < p->prod.id)) {
                p = p->left;
            }
            else {
                p = p->right;
            }
        }

        return nullptr;
    }

    NodeAvl* InPre(NodeAvl* curr) {
        while(curr && curr->right != nullptr) {
            curr = curr->right;
        }
        return curr;
    }

    NodeAvl* InSucc(NodeAvl* curr) {
        while(curr && curr->left != nullptr) {
            curr = curr->left;
        }
        return curr;
    }

    NodeAvl* Delete(NodeAvl* p, string name, int id) {
        if(p == nullptr) {
            return nullptr;
        }

        if(name < p->prod.name ||
           (name == p->prod.name && id < p->prod.id)) {
            p->left = Delete(p->left, name, id);
        }
        else if(name > p->prod.name ||
                (name == p->prod.name && id > p->prod.id)) {
            p->right = Delete(p->right, name, id);
        }
        else {
            if(p->left == nullptr) {
                NodeAvl* temp = p->right;
                delete p;
                return temp;
            }
            else if(p->right == nullptr) {
                NodeAvl* temp = p->left;
                delete p;
                return temp;
            }

            NodeAvl* q;

            if(Height(p->left) > Height(p->right)) {
                q = InPre(p->left);
                p->prod = q->prod;
                p->left = Delete(p->left, q->prod.name, q->prod.id);
            }
            else {
                q = InSucc(p->right);
                p->prod = q->prod;
                p->right = Delete(p->right, q->prod.name, q->prod.id);
            }
        }

        p->height = Height(p);

        if(BalanceFactor(p) == 2 && BalanceFactor(p->left) >= 0) {
            return LLRotation(p);
        }
        else if(BalanceFactor(p) == 2 && BalanceFactor(p->left) == -1) {
            return LRRotation(p);
        }
        else if(BalanceFactor(p) == -2 && BalanceFactor(p->right) <= 0) {
            return RRRotation(p);
        }
        else if(BalanceFactor(p) == -2 && BalanceFactor(p->right) == 1) {
            return RLRotation(p);
        }

        return p;
    }

    void Insert(Product pr) {
        root = Insertion(root, pr);
    }

    void Inorder(NodeAvl* p) {
        if(p != nullptr) {
            Inorder(p->left);
            cout << p->prod.name << " ";
            Inorder(p->right);
        }
    }

    void Delete(string name, int id) {
        root = Delete(root, name, id);
    }

    void Inorder() {
        Inorder(root);
    }

    int GetHeight() {
        return root ? root->height : 0;
    }

    NodeAvl* Search(string name, int id) {
        return Search(root, name, id);
    }

    void destroy(NodeAvl* p) {
        if(p == nullptr) {
            return;
        }
        destroy(p->left);
        destroy(p->right);
        delete p;
    }

    ~AVL() {
        destroy(root);
    }
    AVL(const AVL&) = delete;
    AVL& operator=(const AVL&) = delete;
};

#endif