#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>

#include <ogdf/basic/Graph.h>

class Graph;

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
    ArrayPointers(const int numberOfPointers) {
        pointers_m.resize(numberOfPointers);
        for (int i = 0; i < numberOfPointers; ++i)
            pointers_m[i] = nullptr;
    }
    T* getPointer(const int index) const {
        assert(pointers_m[index] != nullptr);
        return pointers_m[index];
    }
    void setPointer(const int index, T* pointer) {
        assert(pointers_m[index] == nullptr);
        pointers_m[index] = pointer;
    }
};

class OgdfUtils {
public:
    static ogdf::Graph* myGraphToOgdf(const Graph* myGraph);
    static const Graph* ogdfGraphToMyGraph(ogdf::Graph* graph);
    static void printOgdfGraph(ogdf::Graph* graph);
};

inline void saveStringToFile(const char* filename, std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
        std::cout << "File saved successfully." << std::endl;
    } else
        std::cerr << "Unable to open file" << std::endl;
}

#endif