#pragma once

#include <iostream>
#include <vector>
#include "IContainer.cpp"

using namespace std;

template<class TIndex, class TItem, class TCont>
class ConcurrentMap : public IContainer<TIndex, TItem> {
    static_assert(is_base_of<IContainer<TIndex,TItem>, TCont>::value, "TCont must be a IContainer");
public:
    explicit ConcurrentMap(int (*hf)(TIndex&), TCont &container, int bs = 100) 
        : hash_func(hf), size(0), num_buckets(bs) {
        for (int i = 0; i < num_buckets; ++i)
            buckets.push_back(container); // copy container
    }
    
    virtual ~ConcurrentMap() { 
        //for (auto b = buckets.begin(); b != buckets.end(); ++b)
        //    delete b;
    }
    
	virtual int add(TIndex name, TItem elt);
	virtual int query(TIndex name, TItem *item);
	virtual int remove(TIndex elt);
	virtual void print() {
        int i = 0;
        for (auto b = buckets.begin(); b != buckets.end(); ++b) {
            cout << "Bucket #" << i++ << ": ";
            b->print();
        }
    }
private:
    int (*hash_func)(TIndex&);
    int size;
    int num_buckets;
    vector<TCont> buckets;
};

template<class TIndex, class TItem, class TCont>
int ConcurrentMap<TIndex,TItem,TCont>::add(TIndex name, TItem elt) {
    int index = hash_func(name);
    return buckets[index].add(name,elt);
}

template<class TIndex, class TItem, class TCont>
int ConcurrentMap<TIndex,TItem,TCont>::query(TIndex name, TItem *item) {
    int index = hash_func(name);
    return buckets[index].query(name,item);
}

template<class TIndex, class TItem, class TCont> 
int ConcurrentMap<TIndex,TItem,TCont>::remove(TIndex elt) {
    int index = hash_func(elt);
    return buckets[index].remove(elt);
}