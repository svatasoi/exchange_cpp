#include <string>
#include <cstring>
#include <sstream>

#include "message/message.hpp"
#include "server/bid_offer.hpp"

using namespace std;

// -----------------bids and offers--------------------------

bid_offer::bid_offer() : sym("aaaa"), value(-1), volume(-1) {}

bid_offer::bid_offer(const char *msg) {
    client_body_t *body = (client_body_t *)(msg);
    sym = string(body->sym);
    value = body->price;
    volume = body->volume;
    
    
    // stringstream input{string{msg}};
    
    // input >> sym >> value >> volume;
}

bid_offer::bid_offer(string s, double v, int vol) 
    : sym(s), value(v), volume(vol) {}