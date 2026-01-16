#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

// Compare для сравнения (std::less для Max-Heap,для Min-Heap) умолч - std::less
template <typename T, typename Compare = std::less<T>>
class PriorityQueue {
public:
    PriorityQueue();

    void push(const T& value);
    void pop();
    const T& top() const;
    bool empty() const;
    size_t size() const;

private:
    std::vector<T> heap;
    Compare comp;

    // для приоритета
    void siftUp(size_t index);
    void siftDown(size_t index);
};

template <typename T, typename Compare>
PriorityQueue<T, Compare>::PriorityQueue() {}

template <typename T, typename Compare>
void PriorityQueue<T, Compare>::push(const T& value) {
    heap.push_back(value);
    siftUp(heap.size() - 1);
}

template <typename T, typename Compare>
void PriorityQueue<T, Compare>::pop() {
    if (empty()) {
        throw std::out_of_range("PriorityQueue is empty");
    }
    // Перемещаем последний элемент в корень и просеиваем вниз
    heap[0] = heap.back();
    heap.pop_back();

    if (!empty()) {
        siftDown(0);
    }
}

template <typename T, typename Compare>
const T& PriorityQueue<T, Compare>::top() const {
    if (empty()) {
        throw std::out_of_range("PriorityQueue is empty");
    }
    return heap.front();
}

template <typename T, typename Compare>
bool PriorityQueue<T, Compare>::empty() const {
    return heap.empty();
}

template <typename T, typename Compare>
size_t PriorityQueue<T, Compare>::size() const {
    return heap.size();
}

template <typename T, typename Compare>
void PriorityQueue<T, Compare>::siftUp(size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (comp(heap[parent], heap[index])) { // std::less (a<=b) std::great (a>=b)
            std::swap(heap[index], heap[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

template <typename T, typename Compare>
inline void PriorityQueue<T, Compare>::siftDown(size_t index) {
    size_t leftChild, rightChild, largest;

    while (true) {
        leftChild = 2 * index + 1;
        rightChild = 2 * index + 2;
        largest = index;

        if (leftChild < heap.size() && comp(heap[largest], heap[leftChild])) {
            largest = leftChild;
        }

        if (rightChild < heap.size() && comp(heap[largest], heap[rightChild])) {
            largest = rightChild;
        }

        if (largest != index) {
            std::swap(heap[index], heap[largest]);
            index = largest;
        } else {
            break;
        }
    }
}
