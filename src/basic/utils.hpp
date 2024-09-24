#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>

#include <ogdf/basic/Graph.h>

class Graph;

/**
 * @brief Prints the elements of an iterable container to the standard output.
 * 
 * This function takes an iterable container (e.g., vector, list, set) and prints
 * its elements to the standard output in a formatted manner. Each element is 
 * separated by a space and the entire container is enclosed in square brackets.
 * 
 * @tparam Iterable The type of the iterable container.
 * @param container The iterable container whose elements are to be printed.
 * @param end The string to be printed at the end of the output. Defaults to a newline character.
 */
template <typename Iterable>
void printIterable(const Iterable& container, const std::string& end = "\n") {
    std::cout << "[ ";
    for (const auto& elem : container)
        std::cout << elem << " ";
    std::cout << "]" << end;
}

/**
 * @brief Reverses the elements of a vector in place.
 * 
 * This function takes a vector and reverses its elements in place, meaning
 * that the original vector is modified to have its elements in reverse order.
 * 
 * @tparam T The type of elements in the vector.
 * @param vector The vector whose elements are to be reversed.
 */
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

/**
 * @brief Prints the elements of an array to the standard output.
 * 
 * This function takes an array and its size, and prints the elements
 * of the array in a formatted manner. Each element is separated by a space,
 * and the entire array is enclosed in square brackets. An optional string
 * can be provided to specify what to print at the end of the array output.
 * 
 * @tparam T The type of the elements in the array.
 * @param array The array to be printed.
 * @param size The number of elements in the array.
 * @param end The string to be printed at the end of the array output. Defaults to a newline character.
 */
template <typename T>
void printArray(T array[], int size, const std::string& end = "\n") {
    std::cout << "[ ";
    for (int i = 0; i < size; ++i)
        std::cout << array[i] << " ";
    std::cout << "]" << end;
}

/**
 * @class ArrayPointers
 * @brief A template class that manages an array of pointers.
 *
 * This class provides a simple way to manage an array of pointers of type T.
 * It ensures that pointers are initialized to nullptr and provides methods
 * to get and set pointers at specific indices.
 *
 * @tparam T The type of the objects that the pointers will point to.
 */
template <typename T>
class ArrayPointers {
private:
    std::vector<T*> pointers_m;
public:
    /**
     * @brief Constructs an ArrayPointers object with a specified number of pointers.
     *
     * This constructor initializes the internal vector `pointers_m` to the specified
     * size `numberOfPointers` and sets each element to `nullptr`.
     *
     * @param numberOfPointers The number of pointers to be managed by the ArrayPointers object.
     */
    ArrayPointers(const int numberOfPointers) {
        pointers_m.resize(numberOfPointers);
        for (int i = 0; i < numberOfPointers; ++i)
            pointers_m[i] = nullptr;
    }

    /**
     * @brief Retrieves a pointer from the pointers_m array at the specified index.
     * 
     * This function asserts that the pointer at the given index is not null before returning it.
     * 
     * @tparam T The type of the pointer.
     * @param index The index of the pointer to retrieve.
     * @return T* The pointer at the specified index.
     * @throws Assertion failure if the pointer at the specified index is null.
     */
    T* getPointer(const int index) const {
        assert(pointers_m[index] != nullptr);
        return pointers_m[index];
    }

    /**
     * @brief Sets a pointer at the specified index in the pointers array.
     * 
     * This function assigns the given pointer to the specified index in the 
     * pointers_m array. It asserts that the current pointer at the index is 
     * nullptr before assignment.
     * 
     * @tparam T The type of the pointer.
     * @param index The index at which to set the pointer.
     * @param pointer The pointer to be set at the specified index.
     * 
     * @pre The pointer at the specified index must be nullptr.
     */
    void setPointer(const int index, T* pointer) {
        assert(pointers_m[index] == nullptr);
        pointers_m[index] = pointer;
    }
};

/**
 * @class OgdfUtils
 * @brief Utility class for converting and printing graphs using the OGDF library.
 * 
 * This class provides utility static methods for converting between custom Graph objects
 * and OGDF Graph objects, as well as printing OGDF Graph objects.
 */
class OgdfUtils {
public:
    static ogdf::Graph* myGraphToOgdf(const Graph* myGraph);
    static const Graph* ogdfGraphToMyGraph(ogdf::Graph* graph);
    static void printOgdfGraph(ogdf::Graph* graph);
};

/**
 * @brief Saves a string to a file.
 * 
 * This function takes a filename and a string content, and writes the content
 * to the specified file. If the file is successfully opened, the content is 
 * written to the file and a success message is printed to the standard output.
 * If the file cannot be opened, an error message is printed to the standard error.
 * 
 * @param filename The name of the file to which the content will be saved.
 * @param content The string content to be written to the file.
 */
inline void saveStringToFile(std::string filename, std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
        std::cout << "File saved successfully." << std::endl;
    } else
        std::cerr << "Unable to open file" << std::endl;
}

#endif