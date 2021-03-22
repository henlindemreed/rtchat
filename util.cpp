#include "util.hh"

template <typename T>
void remove_unordered(std::vector<T> &v, int i){
    v[i] = v.back();
    v.pop_back();
}