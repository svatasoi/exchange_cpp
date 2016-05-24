#pragma once

#include <string>
#include <cstring>

using namespace std;

#define SYMBOL_LEN 4
typedef string symbol_t;
typedef double amount_t;

// -----------------bids and offers--------------------------

class bid_offer {
public: 
    bid_offer(const char *msg) {
        memcpy(&sym, msg, SYMBOL_LEN);
        memcpy(&value, msg + SYMBOL_LEN, sizeof(amount_t));
        memcpy(&volume, msg + SYMBOL_LEN + sizeof(amount_t), sizeof(int));
    }
          
    bid_offer(string s, double v, int vol) 
        : sym(s), value(v), volume(vol) {}
        
    friend bool operator> (const bid_offer &c1, const bid_offer &c2) {
        return c1.value > c2.value;
    }
    friend bool operator<= (const bid_offer &c1, const bid_offer &c2) {
        return c1.value <= c2.value;
    }
    friend bool operator< (const bid_offer &c1, const bid_offer &c2) {
        return c1.value < c2.value;
    }
    friend bool operator>= (const bid_offer &c1, const bid_offer &c2){
        return c1.value >= c2.value;
    }
protected:
    symbol_t sym;
    amount_t value;
    int      volume;
};

class bid : public bid_offer {
public:
    bid(const char *msg) : bid_offer(msg) {}
    bid(string s, double v, int vol) : bid_offer(s,v,vol) {}
};

class offer : public bid_offer {
public:
    offer(const char *msg) : bid_offer(msg) {}
    offer(string s, double v, int vol) : bid_offer(s,v,vol) {}
};