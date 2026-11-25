#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
using namespace std;

// Hash table with separate chaining. Each entry stores an integer key and a value
// of any type T, so the same class holds customers and products.

template <typename T>
struct HashNode {
    int key;
    T data;
    HashNode<T>* next;
};

template <typename T>
class HashTable {
private:
    HashNode<T>** table;   // array of bucket heads
    int size;              // number of buckets
    int count;             // number of stored entries

    // Maps a key to a bucket index.
    int hash(int key) {
        int h = key % size;
        if (h < 0) h += size;
        return h;
    }

    // Doubles the number of buckets and re-hashes every node into the new array.
    // Nodes are relinked rather than copied.
    void resize() {
        int oldSize = size;
        HashNode<T>** oldTable = table;

        size = size * 2;
        table = new HashNode<T>*[size];
        for (int i = 0; i < size; i++) table[i] = nullptr;

        for (int i = 0; i < oldSize; i++) {
            HashNode<T>* cur = oldTable[i];
            while (cur != nullptr) {
                HashNode<T>* nextNode = cur->next;
                int idx = hash(cur->key);
                cur->next = table[idx];
                table[idx] = cur;
                cur = nextNode;
            }
        }
        delete[] oldTable;
    }

public:
    HashTable(int s = 11) {
        size = (s > 0) ? s : 11;
        count = 0;
        table = new HashNode<T>*[size];
        for (int i = 0; i < size; i++) table[i] = nullptr;
    }

    // Frees every node and then the bucket array.
    ~HashTable() {
        for (int i = 0; i < size; i++) {
            HashNode<T>* cur = table[i];
            while (cur != nullptr) {
                HashNode<T>* nextNode = cur->next;
                delete cur;
                cur = nextNode;
            }
        }
        delete[] table;
    }

    // Copying is disabled because two tables sharing nodes would delete them twice.
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Adds a new key at the head of its bucket, or overwrites the value if the key exists.
    // Resizes when the load factor (entries per bucket) goes above 1.
    void insert(int key, const T& value) {
        T* existing = search(key);
        if (existing != nullptr) {
            *existing = value;
            return;
        }
        int idx = hash(key);
        HashNode<T>* newNode = new HashNode<T>{key, value, table[idx]};
        table[idx] = newNode;
        count++;

        if ((double)count / size > 1.0) resize();
    }

    // Returns a pointer to the stored value, or nullptr if the key is not present.
    T* search(int key) {
        HashNode<T>* cur = table[hash(key)];
        while (cur != nullptr) {
            if (cur->key == key) return &(cur->data);
            cur = cur->next;
        }
        return nullptr;
    }

    // Removes a key. Returns false if it was not found.
    bool remove(int key) {
        int idx = hash(key);
        HashNode<T>* cur = table[idx];
        HashNode<T>* prev = nullptr;
        while (cur != nullptr) {
            if (cur->key == key) {
                if (prev == nullptr) table[idx] = cur->next;
                else prev->next = cur->next;
                delete cur;
                count--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    int getCount() const { return count; }
    int getSize() const { return size; }
    double getLoadFactor() const { return (double)count / size; }

    // Copies every stored value into the given vector.
    void getAll(vector<T>& out) const {
        for (int i = 0; i < size; i++) {
            HashNode<T>* cur = table[i];
            while (cur != nullptr) {
                out.push_back(cur->data);
                cur = cur->next;
            }
        }
    }

    // Prints each bucket and the keys chained in it.
    void displayStructure() const {
        for (int i = 0; i < size; i++) {
            cout << "[" << i << "]";
            HashNode<T>* cur = table[i];
            while (cur != nullptr) {
                cout << " -> " << cur->key;
                cur = cur->next;
            }
            cout << " -> NULL\n";
        }
    }
};

#endif
