#pragma once

#include <string>
#include <cstring>

using namespace std;

#define SYMBOL_LEN 4

typedef string symbol_t;
typedef double amount_t;
typedef size_t token_t;

// -----------------bids and offers--------------------------

class bid_offer {
public: 
    bid_offer();
    bid_offer(const char *msg);
    bid_offer(string s, double v, int vol);
        
    friend bool operator>(const bid_offer &c1, const bid_offer &c2) {
        return c1.value > c2.value;
    }
    friend bool operator<=(const bid_offer &c1, const bid_offer &c2) {
        return c1.value <= c2.value;
    }
    friend bool operator<(const bid_offer &c1, const bid_offer &c2) {
        return c1.value < c2.value;
    }
    friend bool operator>=(const bid_offer &c1, const bid_offer &c2){
        return c1.value >= c2.value;
    }

    symbol_t sym;
    amount_t value;
    int      volume;
    token_t  token;
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