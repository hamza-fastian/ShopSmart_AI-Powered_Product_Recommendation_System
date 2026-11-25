#ifndef MAXHEAP_H
#define MAXHEAP_H

// One heap entry: a product ID and its match score for a customer.
struct ScoredItem {
    int productId;
    double score;
};

// Max-heap stored in an array, so the largest score is always at index 0.
// For the node at index i:
//   parent = (i - 1) / 2,  left child = 2i + 1,  right child = 2i + 2
class MaxHeap {
private:
    ScoredItem* data;
    int capacity;
    int size;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i)   { return 2 * i + 1; }
    int right(int i)  { return 2 * i + 2; }

    void swapItems(int a, int b) {
        ScoredItem temp = data[a];
        data[a] = data[b];
        data[b] = temp;
    }

    // Moves an item up while it is larger than its parent.
    void heapifyUp(int i) {
        while (i > 0 && data[parent(i)].score < data[i].score) {
            swapItems(i, parent(i));
            i = parent(i);
        }
    }

    // Moves an item down, swapping with the larger child, until the heap rule holds.
    void heapifyDown(int i) {
        while (true) {
            int l = left(i);
            int r = right(i);
            int largest = i;
            if (l < size && data[l].score > data[largest].score) largest = l;
            if (r < size && data[r].score > data[largest].score) largest = r;
            if (largest == i) break;
            swapItems(i, largest);
            i = largest;
        }
    }

    // Doubles the array capacity when it is full.
    void grow() {
        capacity *= 2;
        ScoredItem* bigger = new ScoredItem[capacity];
        for (int i = 0; i < size; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
    }

public:
    MaxHeap(int cap = 16) {
        capacity = (cap > 0) ? cap : 16;
        size = 0;
        data = new ScoredItem[capacity];
    }

    ~MaxHeap() { delete[] data; }

    MaxHeap(const MaxHeap&) = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    // Adds the item at the end, then moves it up into place.
    void insert(ScoredItem item) {
        if (size == capacity) grow();
        data[size] = item;
        heapifyUp(size);
        size++;
    }

    // Removes the largest item: the last item replaces the root, then moves down.
    // Returns false if the heap is empty.
    bool extractMax(ScoredItem& out) {
        if (size == 0) return false;
        out = data[0];
        data[0] = data[size - 1];
        size--;
        heapifyDown(0);
        return true;
    }

    bool peek(ScoredItem& out) const {
        if (size == 0) return false;
        out = data[0];
        return true;
    }

    bool isEmpty() const { return size == 0; }
    int getSize() const { return size; }
};

#endif
