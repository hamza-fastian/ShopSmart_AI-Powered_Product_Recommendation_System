#ifndef CIRCULARLL_H
#define CIRCULARLL_H
#include<iostream>
#include "Product.h"
using namespace std;

struct NodeCLL {
    Product product;
    NodeCLL* next;
    NodeCLL* prev;
};

class CircularLL {
private:
    NodeCLL* head;
    NodeCLL* tail;
    NodeCLL* current;

public:
    CircularLL() {
        head = nullptr;
        tail = nullptr;
        current = nullptr;
    }

    ~CircularLL() {
        if(head == nullptr) {
            return;
        }

        NodeCLL* curr = head->next;
        while(curr != head) {
            NodeCLL* temp = curr;
            curr = curr->next;
            delete temp;
        }
        delete head;
        head = nullptr;
        tail = nullptr;
        current = nullptr;
    }
    CircularLL(const CircularLL&) = delete;
    CircularLL& operator=(const CircularLL&) = delete;

    void AddProduct(Product p) {
        if(head != nullptr) {
            NodeCLL* curr = head;

            do {
                if(curr->product.id == p.id) {
                    return;
                }
                curr = curr->next;
            } while(curr != head);
        }

        NodeCLL* newProd = new NodeCLL();
        newProd->product = p;

        if(head == nullptr) {
            head = newProd;
            tail = newProd;

            newProd->next = head;
            newProd->prev = tail;

            current = head;
            return;
        }

        tail->next = newProd;
        newProd->prev = tail;
        newProd->next = head;
        head->prev = newProd;

        tail = newProd;
    }

    void RemoveProduct(int id) {
        if(head == nullptr) {
            return;
        }

        NodeCLL* curr = head;

        do {
            if(curr->product.id == id) {
                if(head == tail) {
                    head = tail = current = nullptr;
                    delete curr;
                    return;
                }

                if(curr == current) {
                    current = curr->next;
                }

                if(curr == head) {
                    head = head->next;
                    head->prev = tail;
                    tail->next = head;
                }
                else if(curr == tail) {
                    tail = tail->prev;
                    tail->next = head;
                    head->prev = tail;
                }
                else {
                    curr->prev->next = curr->next;
                    curr->next->prev = curr->prev;
                }

                delete curr;
                return;
            }

            curr = curr->next;

        } while(curr != head);
    }

    void NextProduct() {
        if(current == nullptr) {
            return;
        }

        current = current->next;
        cout << current->product.name << endl;
    }

    void PreviousProduct() {
        if(current == nullptr) {
            return;
        }

        current = current->prev;
        cout << current->product.name << endl;
    }

    void Display() {
        if(head == nullptr) {
            return;
        }

        NodeCLL* curr = head;

        do {
            cout << curr->product.name << " ";
            curr = curr->next;
        } while(curr != head);

        cout << endl;
    }
};

#endif