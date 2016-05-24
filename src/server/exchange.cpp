#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <boost/asio.hpp>
#include "linked_list/linked_list.cpp"
#include "concurrent_map/concurrent_map.cpp"
#include "exchange.hpp"

using boost::asio::ip::tcp;
using namespace std;

class bid;
class offer;

typedef string symbol_t;
typedef double amount_t;

// -----------------Client--------------------------

Client::Client(client_sess_ptr session) 
    : _session(session), _token(session->get_hash()) {}

Client *Client::connect(client_sess_ptr session) {
    size_t hash = session->get_hash();
    auto cli = _all_clients.find(session->get_hash());
    if (cli == _all_clients.end()) {
        // client doesn't exist
        _all_clients[hash] = new Client(session);
    } else {
        // client already exists
        _all_clients[hash]->_session = session;
    }
    return _all_clients[hash];
}

void Client::disconnect(client_sess_ptr session) {
    _all_clients.erase(session->get_hash());
}

// -----------------bids and offers--------------------------

class bid_offer {
public: 
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

class bid : bid_offer {
public:
    bid(string s, double v, int vol) : bid_offer(s,v,vol) {}
};

class offer : bid_offer {
public:
    offer(string s, double v, int vol) : bid_offer(s,v,vol) {}
};

// ----------------------------------------------------------

// -----------------exchange--------------------------

#define NUM_BUCKETS 20
inline int silly_hash(string &s) {
    return exchange::str_hash(s) % NUM_BUCKETS;
}

inline int compare_str(string& s1, string& s2) {
	return s1.compare(s2);
}

exchange::exchange() {
    /*LinkedList<symbol_t, priority_queue<bid>> list(&compare_str, "", new priority_queue<bid>());
    bids_ = new ConcurrentMap(&silly_hash, list, NUM_BUCKETS); 
    
    LinkedList<symbol_t, priority_queue<offer>> list_o(&compare_str, "", new priority_queue<offer>());
    offers_ = new ConcurrentMap(&silly_hash, list_o, NUM_BUCKETS); */
}

void exchange::join(Client client) {
    // _clients.insert(client);
}