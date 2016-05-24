#pragma once

#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <boost/asio.hpp>
#include "linked_list/linked_list.cpp"
#include "concurrent_map/concurrent_map.cpp"
#include "message/message.hpp"

using boost::asio::ip::tcp;
using namespace std;

class exchange;

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
    void do_read_header();
    void do_read_body();
    void do_write();

    tcp::socket _socket;
    exchange& _exchange;
    message_from_client _client_msg;
//   chat_message_queue write_msgs_;
};

typedef shared_ptr<client_session> client_sess_ptr;

class Client
{
public:    
    // Client() {}
    
    static Client *connect(client_sess_ptr session);
    static void disconnect(client_sess_ptr session);
    
    friend bool operator< (const Client &c1, const Client &c2) {
        return c1._token < c2._token;
    }
private:
    explicit Client(client_sess_ptr session);
    void message_client();
    
    client_sess_ptr _session; // ptr to active session, if there is one
    size_t _token;
    static map<size_t, Client*> _all_clients;
};

class exchange
{
public:
    static hash<string> str_hash;
    
    explicit exchange();
    void join(Client client);
    
    void add_bid();
    void add_offer();
    void get_quote();
private:
    // set<Client> _clients;
    /*
    ConcurrentMap<symbol_t, priority_queue<bid>, LinkedList<symbol_t, priority_queue<bid>>> bids_;
    ConcurrentMap<symbol_t, priority_queue<offer>, LinkedList<symbol_t, priority_queue<offer>>> offers_;
    */
    void message_client();
};