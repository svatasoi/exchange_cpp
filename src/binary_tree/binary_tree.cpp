#pragma once

#include <iostream>
#include <mutex>
#include "IContainer.cpp"
using namespace std;

template<class TIndex, class TItem> class BTNode;

template<class TIndex, class TItem>
class BinaryTree : public IContainer<TIndex,TItem> {
public:
	typedef BTNode<TIndex,TItem> node_t;

	explicit BinaryTree(int (*cmp)(TIndex&,TIndex&), node_t *rt)
			: comparator(cmp), root(rt), size(0) {}
	virtual ~BinaryTree() { delete root; }
	int add(TIndex name, TItem elt);
	int query(TIndex name, TItem *item);
	int remove(TIndex elt);
	void print();
private:
	int (*comparator)(TIndex&,TIndex&);
	node_t *root;
	int size;
	node_t *search(TIndex name, node_t **parent);
};

template<class TIndex, class TItem>
class BTNode {
public:
	explicit BTNode(TIndex i, TItem e, BTNode<TIndex,TItem> *l=NULL, BTNode<TIndex,TItem> *r=NULL) 
		: index(i), item(e), lchild(l), rchild(r) {};

	~BTNode() { delete lchild; delete rchild; }

	void print();

	TIndex index;
	TItem  item;
	BTNode<TIndex,TItem> *lchild;
	BTNode<TIndex,TItem> *rchild;
	mutex mtx;

	inline void lock() { mtx.lock(); }
	inline void unlock() { mtx.unlock(); }
private:
};

// BinaryTree functions
template<class TIndex, class TItem>
int BinaryTree<TIndex,TItem>::add(TIndex name, TItem elt) {
	BTNode<TIndex,TItem> *parent;
	BTNode<TIndex,TItem> *target = search(name, &parent);
	
	if(target) { // Name is already in BinaryTree
		parent->unlock();
		target->unlock();
		return 0;
	}

	target = new BTNode<TIndex,TItem>(name, elt);
	++size;

	if(comparator(target->index, parent->index)<0)
		parent->lchild = target;
	else
		parent->rchild = target;
	
	parent->unlock();
	return 1;
}

template<class TIndex, class TItem>
BTNode<TIndex,TItem> *BinaryTree<TIndex,TItem>::search(TIndex name, BTNode<TIndex,TItem> **parent) {
	BTNode<TIndex,TItem> *next = root;
	*parent = NULL;
	do {
		next->lock(); 
		if (*parent)
			(*parent)->unlock();

		*parent = next;
		if(comparator(name, next->index)<0)
			next = (*parent)->lchild;
		else
			next = (*parent)->rchild;
	} while (next && comparator(name, next->index));
	if (next)
		next->lock();
	return next;
}

template<class TIndex, class TItem>
int BinaryTree<TIndex,TItem>::query(TIndex name, TItem *item) {
	BTNode<TIndex,TItem> *parent;
	BTNode<TIndex,TItem> *node = search(name, &parent);
	if (node) {
		*item = node->item;
		parent->unlock();
		node->unlock();
		return 1;
	}
	return 0;
}

template<class TIndex, class TItem>
int BinaryTree<TIndex,TItem>::remove(TIndex name) {
	BTNode<TIndex,TItem> *parent;
	BTNode<TIndex,TItem> *target = search(name, &parent);
	
	if(!target) { // Name is already in BinaryTree
		parent->unlock();
		return 0;
	}

	BTNode<TIndex,TItem> *tleft = target->lchild, *tright = target->rchild;

	BTNode<TIndex,TItem> *successor;
	if (!tleft) {
		if (tright) tright->lock();
		successor = tright;
	} else if (!tright) {
		if (tleft) tleft->lock();
		successor = tleft;
	} else {
		// go to rchild, then all the way to left
		BTNode<TIndex,TItem> *successor_parent = NULL;
		tright->lock();

		successor = tright;
		while (successor->lchild) {
			successor_parent = successor;
			successor = successor->lchild;

			successor->lock(); 
			successor_parent->unlock();
		}
		if (successor_parent) {
			successor_parent->lock();
			successor_parent->lchild = successor->rchild;
			successor->rchild = tright;
			successor_parent->unlock();
		}

		successor->lchild = tleft;
	}

	--size;
	if(comparator(name, parent->index)<0)
		parent->lchild = successor;
	else
		parent->rchild = successor;

	target->lchild = target->rchild = NULL;
	// target->unlock();
	delete target;

	if (successor)
		successor->unlock();
	parent->unlock();
	return 1;
}

template<class TIndex, class TItem>
void BinaryTree<TIndex,TItem>::print() {
	BTNode_rprint(root, 0);
}

static inline void print_spaces(int nspaces);

template<class TIndex, class TItem>
static void BTNode_rprint(BTNode<TIndex,TItem> *node, int level) {
	print_spaces(level);
	if(!node) {
		cout << "(null)" << endl;
		return;
	}
	node->print();
	if(node) {
		BTNode_rprint(node->lchild, level+1);
		BTNode_rprint(node->rchild, level+1);
	}
}

/* Print the given number of spaces */
static inline void print_spaces(int nspaces) {
	while(0 < nspaces--)
		cout << " ";
}

// BTNode functions
template<class TIndex, class TItem>
void BTNode<TIndex,TItem>::print() {
	cout << index << " " << item << endl;
}