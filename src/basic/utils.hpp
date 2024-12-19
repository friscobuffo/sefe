#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <fstream>
#include <memory>
#include <iterator>

#include <ogdf/basic/Graph.h>

template <typename Iterable>
void printIterable(const Iterable& container, const std::string& end = "\n") {
    std::cout << "[ ";
    for (const auto& elem : container)
        std::cout << elem << " ";
    std::cout << "]" << end;
}

template <typename T>
void reverseVector(std::vector<T>& vector) {
    int start = 0;
    int end = vector.size()-1;
    while (start < end) {
        T temp = vector[start];
        vector[start] = vector[end];
        vector[end] = temp;
        ++start;
        --end;
    }
}

template <typename T>
void printArray(T array[], int size, const std::string& end = "\n") {
    std::cout << "[ ";
    for (int i = 0; i < size; ++i)
        std::cout << array[i] << " ";
    std::cout << "]" << end;
}

template <typename T>
class ArrayPointers {
private:
    std::vector<T*> pointers_m;
public:
    T* getPointer(const int index) const {
        assert(index >= 0 && index < pointers_m.size());
        assert(pointers_m[index] != nullptr);
        return pointers_m[index];
    }

    void setPointer(const int index, T* pointer) {
        assert(index >= 0);
        while (pointers_m.size() <= index)
            pointers_m.push_back(nullptr);
        assert(pointers_m[index] == nullptr);
        pointers_m[index] = pointer;
    }

    void resetPointer(const int index) {
        assert(index >= 0 && index < pointers_m.size());
        assert(&pointers_m[index] != nullptr);
        pointers_m[index] = nullptr;
    }
};

inline void saveStringToFile(std::string filename, std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
    } else
        std::cerr << "Unable to open file" << std::endl;
}

template <typename T>
class PointersIterator {
private:
    using InnerIterator = typename std::vector<std::unique_ptr<T>>::iterator;
    InnerIterator m_inner;
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    PointersIterator(InnerIterator inner) : m_inner(inner) {}

    reference operator*() {
        return **m_inner;
    }

    reference operator*() const {
        return **m_inner;
    }

    PointersIterator& operator++() {
        ++m_inner;
        return *this;
    }

    bool operator==(const PointersIterator& other) const {
        return m_inner == other.m_inner;
    }

    bool operator!=(const PointersIterator& other) const {
        return m_inner != other.m_inner;
    }

    reference operator[](difference_type n) {
        return *(*this + n);
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }
};

template <typename T>
class ConstPointersIterator {
private:
    using InnerIterator = typename std::vector<std::unique_ptr<T>>::const_iterator;
    InnerIterator m_inner;
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    ConstPointersIterator(InnerIterator inner) : m_inner(inner) {}

    reference operator*() const {
        return **m_inner;
    }

    ConstPointersIterator& operator++() {
        ++m_inner;
        return *this;
    }

    bool operator==(const ConstPointersIterator& other) const {
        return m_inner == other.m_inner;
    }

    bool operator!=(const ConstPointersIterator& other) const {
        return m_inner != other.m_inner;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }
};

template <typename T>
class Container {
private:
    std::vector<std::unique_ptr<T>> m_vector;

public:
    PointersIterator<T> begin() {
        return PointersIterator<T>(m_vector.begin());
    }

    PointersIterator<T> end() {
        return PointersIterator<T>(m_vector.end());
    }

    ConstPointersIterator<T> cbegin() const {
        return ConstPointersIterator<T>(m_vector.cbegin());
    }

    ConstPointersIterator<T> cend() const {
        return ConstPointersIterator<T>(m_vector.cend());
    }

    ConstPointersIterator<T> begin() const {
        return cbegin();
    }

    ConstPointersIterator<T> end() const {
        return cend();
    }

    void addElement(std::unique_ptr<T> elem) {
        m_vector.push_back(std::move(elem));
    }

    T& operator[](size_t index) {
        return *(m_vector[index]);
    }

    const T& operator[](size_t index) const {
        return *(m_vector[index]);
    }

    size_t size() const {
        return m_vector.size();
    }
};

class Graph;

class OgdfUtils {
public:
    static std::unique_ptr<ogdf::Graph> myGraphToOgdf(const Graph& myGraph);
    static std::unique_ptr<const Graph> ogdfGraphToMyGraph(ogdf::Graph& graph);
    static void printOgdfGraph(ogdf::Graph& graph);
};

#endif