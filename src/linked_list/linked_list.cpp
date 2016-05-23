#pragma once

#include <iostream>
#include <mutex>
#include "IContainer.cpp"
using namespace std;

template<class TIndex, class TItem> class LLNode;

template<class TIndex, class TItem>
class LinkedList : public IContainer<TIndex, TItem> {
public:
	typedef LLNode<TIndex,TItem> node_t;

	explicit LinkedList(int (*cmp)(TIndex&,TIndex&), TIndex rin, TItem rit)
			: comparator(cmp), root(new LLNode<TIndex,TItem>(rin,rit)), size(0) {}
	LinkedList(const LinkedList<TIndex,TItem> &other);
	LinkedList<TIndex,TItem>& operator=(const LinkedList<TIndex,TItem> &other);
	virtual ~LinkedList() { delete root; }
    
	virtual int add(TIndex name, TItem elt);
	virtual int query(TIndex name, TItem *item);
	virtual int remove(TIndex elt);
	virtual void print();
private:
	int (*comparator)(TIndex&,TIndex&);
	node_t *root;
	int size;
	node_t *search(TIndex name, node_t **parent);
};

template<class TIndex, class TItem> 
LinkedList<TIndex,TItem>::LinkedList(const LinkedList<TIndex,TItem> &other) {
	comparator = other.comparator;
	root = new LLNode<TIndex,TItem>(other.root->index, other.root->item);
	size = 1;
}

template<class TIndex, class TItem> 
LinkedList<TIndex,TItem>& LinkedList<TIndex,TItem>::operator=(const LinkedList<TIndex,TItem> &other) {
	auto newList = new LinkedList<TIndex,TItem>(other.comparator, other.root->index, other.root->item);
	return *newList;
}

template<class TIndex, class TItem> 
class LLNode {
public:
	explicit LLNode(TIndex i, TItem e, LLNode<TIndex,TItem> *nxt=nullptr) 
		: index(i), item(e), next(nxt) {};
		
	LLNode<TIndex,TItem>& operator=(const LLNode<TIndex,TItem> &other) {
		auto newNode = new LLNode<TIndex,TItem>(other.index, other.item);
		return &newNode;
	};

	~LLNode() { if (next) delete next; }

	void print();

	TIndex index;
	TItem  item;
	LLNode<TIndex,TItem> *next;
	mutex mtx;

	inline void lock() { mtx.lock(); }
	inline void unlock() { mtx.unlock(); }
};

template<class TIndex, class TItem>
int LinkedList<TIndex,TItem>::add(TIndex name, TItem elt) {
	LLNode<TIndex,TItem> *parent;
	LLNode<TIndex,TItem> *target = search(name, &parent);
	
	if(target) { // Name is already in list
		target->unlock();
		if (parent) parent->unlock();
		return 0;
	}

	target = new LLNode<TIndex,TItem>(name, elt);
	parent->next = target;
	++size;
	
	if (parent) parent->unlock();
	return 1;
}

template<class TIndex, class TItem>
int LinkedList<TIndex,TItem>::query(TIndex name, TItem *elt) {
	LLNode<TIndex,TItem> *parent;
	LLNode<TIndex,TItem> *target = search(name, &parent);
	
	if(target) { // Name is already in list
        *elt = target->item;
		target->unlock();
		if (parent) parent->unlock();
		return 1;
	}

	if (parent) parent->unlock();
	return 0;
}

template<class TIndex, class TItem>
int LinkedList<TIndex,TItem>::remove(TIndex name) {
    LLNode<TIndex,TItem> *parent;
	LLNode<TIndex,TItem> *target = search(name, &parent);
	
	if(!target) { // Name is already in list
		if (parent) parent->unlock();
		return 0;
	}
    
    // found element to remove
    if (parent) parent->next = target->next;
    target->next = nullptr;
    delete target;

	if (parent) parent->unlock();
	return 1;
}

template<class TIndex, class TItem>
LLNode<TIndex,TItem> *LinkedList<TIndex,TItem>::search(TIndex name, LLNode<TIndex,TItem> **parent) {
    LLNode<TIndex,TItem> *curr = root;
	*parent = nullptr;
    if (!curr) return nullptr;
    
    curr->lock(); 
	while (curr && comparator(name, curr->index)) {		            
		if (*parent)
			(*parent)->unlock();

		*parent = curr;
        
		if (curr->next) curr->next->lock(); 
        curr = curr->next;
	} 
	return curr;
}

template<class TIndex, class TItem>
void LinkedList<TIndex,TItem>::print() {
	rprint(root);
}

template<class TIndex, class TItem> 
void LLNode<TIndex,TItem>::print() {
	cout << "[" << index << ": " << item << "]";
}

template<class TIndex, class TItem> 
static void rprint(LLNode<TIndex,TItem> *node) {
	if (!node)
		cout << "END" << endl;
	else {
		node->print();
		cout << " -> ";
		rprint(node->next);
	}
}