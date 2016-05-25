#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <future>
#include <boost/asio.hpp>

#include "bid_offer.hpp"
#include "server/exchange.hpp"
#include "message/message.hpp"

using boost::asio::ip::tcp;
using namespace std;

map<token_t, Client*> Client::_all_clients;

// -----------------client_session-----------------

size_t client_session::get_hash() {
    tcp::endpoint endpoint = _socket.remote_endpoint();
    std::ostringstream stream;
    stream << endpoint;
    std::hash<std::string> hasher;
    return hasher(stream.str());
}

void client_session::start() {
    cout << "Connecting to new client..." << endl;
    // register as client to exchange
    Client *c = Client::connect(shared_from_this());
    
    // join the exchange
    _exchange.join(c);
    _client = c;
    
    // start reading from client
    async(std::launch::async, [this](){ do_read_header(); });
    cout << "Connected to new client!" << endl;
}

void client_session::terminate() {
    cout << "Terminating client session" << endl;
    Client::disconnect(shared_from_this());
}

void client_session::deliver(const message_from_server& msg)
{
    // bool write_in_progress = !write_msgs_.empty();
    // write_msgs_.push_back(msg);
    // if (!write_in_progress)
    // {
    //     do_write();
    // }
}

void client_session::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(_socket,
        boost::asio::buffer(_client_msg.data(), message::header_length),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec && _client_msg.decode_header())
            {
                do_read_body();
            }
            else
            {
                terminate();
            }
        });
}

void client_session::do_read_body()
{
    auto self(shared_from_this());
        boost::asio::async_read(_socket,
        boost::asio::buffer(_client_msg.body(), _client_msg.body_length()),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                // process header+body (stored in _client_msg)
                // pass header,body to handle_message
                try {
                    int err = handle_message(_client_msg.data(), _client_msg.body());
                    if (err < 0)
                        terminate();
                    else
                        do_read_header();
                }
                catch (...) {
                    terminate();
                }
            }
            else
            {
                terminate();
            }
        });
}

int client_session::handle_message(const char *header, const char *body) {
    client_header_t head = static_cast<client_header_t>(header);
    switch (head.type) {
    case BID:
        _exchange.add_bid(static_cast<bid>(body), _client->get_token());
        break;
    case OFFER:
        _exchange.add_offer(static_cast<offer>(body), _client->get_token());
        break;
    default:
        throw "Unknown type";
    };
    return 0;
}

void client_session::do_write()
{
    // auto self(shared_from_this());
    // boost::asio::async_write(socket_,
    //     boost::asio::buffer(write_msgs_.front().data(),
    //         write_msgs_.front().length()),
    //     [this, self](boost::system::error_code ec, std::size_t)
    //     {
    //         if (!ec)
    //         {
    //             write_msgs_.pop_front();
    //             if (!write_msgs_.empty())
    //             {
    //                 do_write();
    //             }
    //         }
    //         else
    //         {
    //             terminate();
    //         }
    //     });
}

// -----------------Client--------------------------

Client::Client(client_sess_ptr session) 
    : _session(session), _token(session->get_hash()) {}

token_t Client::get_token() {
    return _token;
}

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

// -----------------exchange--------------------------

#define NUM_BUCKETS 20
inline int silly_hash(string &s) {
    return exchange::str_hash(s) % NUM_BUCKETS;
}

inline int compare_str(string& s1, string& s2) {
	return s1.compare(s2);
}

void exchange::add_bid(bid b, token_t tok) {
    cout << "Added bid to exchange" << endl;
    b.token = tok;
    _bids[b.sym].push(b);
}

void exchange::add_offer(offer o, token_t tok) {
    cout << "Added offer to exchange" << endl;
    o.token = tok;
    _offers[o.sym].push(o);
}
    
exchange::exchange() {
    /*LinkedList<symbol_t, priority_queue<bid>> list(&compare_str, "", new priority_queue<bid>());
    bids_ = new ConcurrentMap(&silly_hash, list, NUM_BUCKETS); 
    
    LinkedList<symbol_t, priority_queue<offer>> list_o(&compare_str, "", new priority_queue<offer>());
    offers_ = new ConcurrentMap(&silly_hash, list_o, NUM_BUCKETS); */
}

void exchange::join(Client *client) {
    _clients.insert(client);
}