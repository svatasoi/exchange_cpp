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
//   chat_message_queue write_msgs_;
};

typedef shared_ptr<client_session> client_sess_ptr;

class Client
{
public:    
    // Client() {}
    
    static Client *connect(client_sess_ptr session);
    static void disconnect(client_sess_ptr session);
    
    token_t get_token();
    
    friend bool operator< (const Client &c1, const Client &c2) {
        return c1._token < c2._token;
    }
private:
    explicit Client(client_sess_ptr session);
    void message_client();
    
    client_sess_ptr _session; // ptr to active session, if there is one
    token_t _token;
    static map<token_t, Client*> _all_clients;
    static mutex _mtx;
};

template<class T>
class safe_queue : public priority_queue<T> {
public:
    safe_queue() { mtx = new mutex(); }
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
    void check_matches(symbol_t sym);
    void get_quote();
private:
    set<Client *> _clients;
    void match(bid &b, offer &o);
    
    // NOT YET THREAD SAFE
    // also, one of these needs to be in reverse order!
    // bids should be max-heap, offers should be min-heap
    map<symbol_t, safe_queue<bid>> _bids;
    map<symbol_t, safe_queue<offer>> _offers;
    
    void message_client();
};