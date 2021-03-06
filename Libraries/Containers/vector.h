//
// Created by johno on 4/27/2020.
//

#ifndef CSSCUSTOMCONTROLS_VECTOR_H
#define CSSCUSTOMCONTROLS_VECTOR_H

#include "Memory.h"
#include "stddef.h"
#include "CLibs/InitializerList.h"




template<class T>
class vector {
public:
    vector();
    explicit vector(size_t size);
    vector(size_t size, const T& val);
    vector(const vector<T>& other);
    vector(std::initializer_list<T> list);
    ~vector<T>();

    T& operator[](int index) const;
    bool operator==(const vector<T>& other);
    bool operator!=(const vector<T>& other);
    /*bool operator<(const vector<T>& other);
    bool operator>(const vector<T>& other);
    bool operator<=(const vector<T>& other);
    bool operator>=(const vector<T>& other);*/
    void push(const T& val);
    void pop_back();
    size_t size() const;
    void reallocate(size_t newSize);
    //void insert(size_t index, const T& val);
    //void erase(size_t index);
    //destroys elements from start, up to (but not including) end
    //void erase(size_t start, size_t end);
    void clear();

    T* allocate(size_t newSize);

    size_t index(const T& val);
    //size_t findIf(bool (*condition)(const T& x));

private:
    size_t length = 0;
    size_t maxLength = 10;
    T* Array;
};


template<class T>
T& vector<T>::operator[](int index) const{
    if (index < 0) {
        index += length;
    }
    return Array[index];
}


template<class T>
vector<T>::~vector<T>() {
    delete[] Array;
}


template<class T>
bool vector<T>::operator==(const vector<T>& other) {
    if (size() != other.size()) {
        return false;
    }
    for (int i = 0; i < size(); i++) {
        if (*this[i] != other[i]) {
            return false;
        }
    }
    return true;
}


template<class T>
bool vector<T>::operator!=(const vector<T>& other) {
    return !(*this == other);
}


/*bool vector<T>::operator<(const vector<T>& other);
bool vector<T>::operator>(const vector<T>& other);
bool vector<T>::operator<=(const vector<T>& other);
bool vector<T>::operator>=(const vector<T>& other);*/


template<class T>
void vector<T>::push(const T& val) {
    if (length >= maxLength) {
        reallocate(maxLength * 2);
    }
    Array[length] = val;
    length++;
}

template<class T>
void vector<T>::pop_back() {
    (Array + length)->~T();
    length--;
}

template<class T>
size_t vector<T>::size() const {
    return length;
}



template<class T>
void vector<T>::reallocate(size_t newSize) {
    T* temp = allocate(newSize);
    if(newSize < length) {
        length = newSize;
    }
    for(int i = 0; i < length; i++) {
        temp[i] = Array[i];
    }
    delete[] Array;
    Array = temp;
    maxLength = newSize;
}


template<class T>
size_t vector<T>::index(const T& val) {
    for (size_t i = 0; i < this->size(); i++) {
        if (val == *this[i]) {
            return i;
        }
    }
    return -1;
}


/*template<class T>
size_t vector<T>::findIf(bool (*condition)(const T& x)) {
    for (size_t i = 0; i < this->size(); i++) {
        if (condition(*this[i])) {
            return i;
        }
    }
    return this->size();
}*/

template<class T>
void vector<T>::clear() {
    for(int i = 0; i < length; i++) {
        (Array + i)->~T();
    }
    length = 0;
}

template<class T>
T *vector<T>::allocate(size_t newSize) {
    return (T*)malloc(sizeof(T) * newSize);
}

template<class T>
vector<T>::vector() {
    Array = allocate(maxLength);
}

template<class T>
vector<T>::vector(size_t size) {
    maxLength = size;
    Array = allocate(maxLength);
}

template<class T>
vector<T>::vector(size_t size, const T &val) {
    maxLength = size;
    Array = allocate(maxLength);
    for(int i = 0; i < size; i++) {
        Array[i] = val;
    }
}

template<class T>
vector<T>::vector(const vector<T> &other) {
    maxLength = other.size();
    length = other.size();
    Array = allocate(maxLength);
    for(int i = 0; i < other.size(); i++) {
        Array[i] = other[i];
    }
}

template<class T>
vector<T>::vector(std::initializer_list<T> list) {
    Array = allocate(maxLength);
    auto x = list.begin();
    while(x != list.end()) {
        push(*x);
        x++;
    }
}

#endif //CSSCUSTOMCONTROLS_VECTOR_H
