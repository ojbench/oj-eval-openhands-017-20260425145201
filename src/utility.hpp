#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <iostream>
#include <cstring>
#include <string>
namespace sjtu {
template<class T1, class T2>
class pair {
public:
    T1 first;
    T2 second;
    pair() : first(), second() {}
    pair(const T1 &a, const T2 &b) : first(a), second(b) {}
    template<class U1, class U2>
    pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
    bool operator<(const pair &other) const {
        if (first < other.first) return true;
        if (other.first < first) return false;
        return second < other.second;
    }
    bool operator==(const pair &other) const {
        return first == other.first && second == other.second;
    }
};
template<class T>
void swap(T &a, T &b) {
    T tmp = a;
    a = b;
    b = tmp;
}
template<class T>
class vector {
    T *data;
    size_t sz;
    size_t cap;
    void reserve(size_t new_cap) {
        if (new_cap <= cap) return;
        T *new_data = (T*)operator new(new_cap * sizeof(T));
        for (size_t i = 0; i < sz; ++i) {
            new (new_data + i) T(data[i]);
            data[i].~T();
        }
        if (data) operator delete(data);
        data = new_data;
        cap = new_cap;
    }
public:
    vector() : data(nullptr), sz(0), cap(0) {}
    vector(const vector &other) : data(nullptr), sz(0), cap(0) {
        reserve(other.sz);
        for (size_t i = 0; i < other.sz; ++i) new (data + i) T(other.data[i]);
        sz = other.sz;
    }
    ~vector() {
        for (size_t i = 0; i < sz; ++i) data[i].~T();
        if (data) operator delete(data);
    }
    vector &operator=(const vector &other) {
        if (this == &other) return *this;
        for (size_t i = 0; i < sz; ++i) data[i].~T();
        sz = 0;
        reserve(other.sz);
        for (size_t i = 0; i < other.sz; ++i) new (data + i) T(other.data[i]);
        sz = other.sz;
        return *this;
    }
    void push_back(const T &val) {
        if (sz == cap) reserve(cap == 0 ? 1 : cap * 2);
        new (data + sz) T(val);
        sz++;
    }
    size_t size() const { return sz; }
    T &operator[](size_t idx) { return data[idx]; }
    const T &operator[](size_t idx) const { return data[idx]; }
    void clear() {
        for (size_t i = 0; i < sz; ++i) data[i].~T();
        sz = 0;
    }
};
template<int N>
class FixedString {
    char data[N + 1];
public:
    FixedString() { data[0] = '\0'; }
    FixedString(const char *s) { std::strncpy(data, s, N); data[N] = '\0'; }
    FixedString(const std::string &s) { std::strncpy(data, s.c_str(), N); data[N] = '\0'; }
    bool operator<(const FixedString &other) const { return std::strcmp(data, other.data) < 0; }
    bool operator==(const FixedString &other) const { return std::strcmp(data, other.data) == 0; }
    bool operator<=(const FixedString &other) const { return std::strcmp(data, other.data) <= 0; }
    const char* c_str() const { return data; }
};
template<class T, class Compare>
void sort(T *begin, T *end, Compare comp) {
    if (begin >= end) return;
    T *i = begin, *j = end - 1;
    T pivot = begin[(end - begin) / 2];
    while (i <= j) {
        while (comp(*i, pivot)) i++;
        while (comp(pivot, *j)) j--;
        if (i <= j) { swap(*i, *j); i++; j--; }
    }
    if (begin < j + 1) sort(begin, j + 1, comp);
    if (i < end) sort(i, end, comp);
}
}
#endif
