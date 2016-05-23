#pragma once

template<class TIndex, class TItem>
class IContainer {
public:    
	virtual int add(TIndex name, TItem elt) = 0;
	virtual int query(TIndex name, TItem *item) = 0;
	virtual int remove(TIndex elt) = 0;
    virtual void print() = 0;
};