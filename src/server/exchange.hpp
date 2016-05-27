#pragma once

#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>

#include "bid_offer.hpp"
#include "message/message.hpp"

using boost::asio::ip::tcp;
using namespace std;

class exchange;
class Client;

// ------------------client_session----------------------------------

class client_session
  : public std::enable_shared_from_this<client_session>
{
public:
    static hash<tcp::socket> socket_hash;

    client_session(tcp::socket socket, exchange& exc)
        : _socket(std::move(socket)),
          _exchange(exc) {}

    size_t get_hash();
    void start();
    void terminate();
    void deliver(const message_from_server& msg);
private:
    int handle_message(const char *header, const char *body);
    void do_read_header();
    void do_read_body();
    void do_write();

    tcp::socket _socket;
    exchange& _exchange;
    message_from_client _client_msg;
    Client *_client;
    deque<message_from_server> _write_msgs;
};

typedef shared_ptr<client_session> client_sess_ptr;

class Client
{
public:    
    // Client() {}
    
    static Client *connect(client_sess_ptr session);
    static void disconnect(client_sess_ptr session);
    
    token_t get_token();
    void deliver(const message_from_server& msg);
    
    friend bool operator< (const Client &c1, const Client &c2) {
        return c1._token < c2._token;
    }
    
    static map<token_t, Client*> all_clients;
private:
    explicit Client(client_sess_ptr session);
    void message_client();
    
    client_sess_ptr _session; // ptr to active session, if there is one
    token_t _token;
    static mutex _mtx;
};

template<class T, class T2=vector<T>, class T3=less<T>>
class safe_queue : public priority_queue<T,T2,T3> {
public:
    safe_queue() { mtx = new mutex(); }
    ~safe_queue() { delete mtx; }
    void lock() { mtx->lock(); }
    void unlock() { mtx->unlock(); }
    mutex *mtx;
};

class exchange
{
public:
    static hash<string> str_hash;
    
    explicit exchange();
    void join(Client *client);
    
    void add_bid(bid b, token_t tok);
    void add_offer(offer b, token_t tok);
    void buy(token_t tok, symbol_t sym, int volume);
    void sell(token_t tok, symbol_t sym, int volume);
    void check_matches(symbol_t sym);
    bool get_quote(symbol_t sym, bid *b, offer *o);
private:
    void match(token_t buyer_tok, token_t seller_tok, symbol_t sym, double price, int volume);
    void notify_client_of_match(token_t tok, symbol_t sym, double price, int volume, bool is_buyer);
 
    // NOT YET THREAD SAFE
    // also, one of these needs to be in reverse order!
    // bids should be max-heap, offers should be min-heap
    map<symbol_t, safe_queue<bid>> _bids;
    map<symbol_t, safe_queue<offer, vector<offer>, greater<offer>>> _offers;
    
    void message_client();
};