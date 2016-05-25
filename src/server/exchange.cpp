#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <future>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>

#include "bid_offer.hpp"
#include "server/exchange.hpp"
#include "message/message.hpp"

using boost::asio::ip::tcp;
using namespace std;

map<token_t, Client*> Client::_all_clients;
mutex Client::_mtx;

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
                do_read_body(); // couldn't this cause stack overflow? mutually recursive
            }
            else
            {
                cout << "Error reading message header: " << _client_msg.data() << endl;
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
                    if (err < 0) {
                        cout << "Error handling message" << endl;
                        terminate();
                    } else
                        do_read_header();
                }
                catch (...) {
                    cout << "Exception handling message" << endl;
                    terminate();
                }
            }
            else
            {
                cout << " Error reading body" << endl;
                terminate();
            }
        });
}


/* Sequential read :(
void client_session::do_read_header()
{
    boost::system::error_code ec;
    boost::asio::read(_socket, boost::asio::buffer(_client_msg.data(), message::header_length), ec);
        
    if (!ec && _client_msg.decode_header())
    {
        do_read_body();
    }
    else
    {
        cout << "Error reading message header [" << ec << "]: " << ec.message() << " | " << _client_msg.data() << endl;
        terminate();
    }
}

void client_session::do_read_body()
{
    boost::system::error_code ec;
    cout << "Bout to read " << _client_msg.body_length() << " bytes" << endl;
    boost::asio::read(_socket, boost::asio::buffer(_client_msg.body(), _client_msg.body_length()), ec);
    if (!ec)
    {
        // process header+body (stored in _client_msg)
        // pass header,body to handle_message
        try {
            int err = handle_message(_client_msg.data(), _client_msg.body());
            if (err < 0) {
                cout << "Error handling message" << endl;
                terminate();
            } else
                do_read_header();
        }
        catch (...) {
            cout << "Exception handling message" << endl;
            terminate();
        }
    }
    else
    {
        cout << " Error reading body" << endl;
        terminate();
    }
}
*/

int client_session::handle_message(const char *head, const char *body) {
    // client_header_t head = static_cast<client_header_t>(header);
    client_header_t *header = (client_header_t *)head;
    switch (header->type) {
    case BID:
        _exchange.add_bid(static_cast<bid>(body), _client->get_token());
        break;
    case OFFER:
        _exchange.add_offer(static_cast<offer>(body), _client->get_token());
        break;
    default:
        throw "Unknown type";
    };
    
    // must work (fix later)
    bid_offer bo = static_cast<bid_offer>(body);
    _exchange.check_matches(bo.sym);
    
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
    _mtx.lock();
    size_t hash = session->get_hash();
    auto cli = _all_clients.find(session->get_hash());
    if (cli == _all_clients.end()) {
        // client doesn't exist
        _all_clients[hash] = new Client(session);
        cout << "New client (" << _all_clients[hash]->get_token() << ") has connected" << endl;
    } else {
        // client already exists
        _all_clients[hash]->_session = session;
        cout << "Welcome back (" << _all_clients[hash]->get_token() << ")" << endl;
    }
    _mtx.unlock();
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
    b.token = tok;
    _bids[b.sym].lock();
    _bids[b.sym].push(b);
    _bids[b.sym].unlock();
    cout << "[client " << tok << "]: " << b.volume << " " << b.sym << " bid @ $" << b.value << endl;
}

void exchange::add_offer(offer o, token_t tok) {
    o.token = tok;
    _offers[o.sym].lock();
    _offers[o.sym].push(o);
    _offers[o.sym].unlock();
    cout << "[client " << tok << "]: " << o.volume << " " << o.sym << " offer @ $" << o.value << endl;
}

void exchange::check_matches(symbol_t sym) {
    // try to match bid+offer
    // get symbol_t
    // lock both priority_queue's
    // while best_bid >= best_offer (and not same buyer/seller)
    // match best to best
    auto &bid_q = _bids[sym];
    auto &offer_q = _offers[sym];
    
    bid_q.lock();
    offer_q.lock();
    
    if (bid_q.empty() || offer_q.empty()) {
        offer_q.unlock();
        bid_q.unlock();
        return;
    }
        
    // volume!!
    // lock
    bid matched_bid;
    offer matched_offer;
    while (!bid_q.empty() && !offer_q.empty() 
        && (matched_bid = bid_q.top()) >= (matched_offer = offer_q.top())) {
        bid_q.pop();
        offer_q.pop();
        match(matched_bid, matched_offer);
    }
    offer_q.unlock();
    bid_q.unlock();
    // unlock
}

void exchange::match(bid &b, offer &o) {
    cout << "Matched bid " << b.value << " and offer " << o.value << endl;
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