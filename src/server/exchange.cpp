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

map<token_t, Client*> Client::all_clients;
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
    bool write_in_progress = !_write_msgs.empty();
    _write_msgs.push_back(msg);
    if (!write_in_progress)
    {
        do_write();
    }
}

void client_session::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(_socket,
        boost::asio::buffer(_client_msg.data(), message_from_client::header_length),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec && _client_msg.decode_header())
            {
                if (_client_msg.is_exit_msg()) {
                    cout << "Client Exiting" << endl;
                    terminate();
                } else 
                    do_read_body(); 
                    // couldn't this cause stack overflow? mutually recursive
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
                    } else if (err == 0) {
                        cout << "Client exited" << endl;
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
                cout << "Error reading body" << endl;
                terminate();
            }
        });
}

int client_session::handle_message(const char *head, const char *body) {
    client_header_t *header = (client_header_t *)head;
    token_t tok = _client->get_token();
    switch (header->type) {
    case BID:
        _exchange.add_bid(static_cast<bid>(body), tok);
        break;
    case OFFER:
        _exchange.add_offer(static_cast<offer>(body), tok);
        break;
    case BUY:
        {
            client_buysell_body_t *q = (client_buysell_body_t *)(body);
            _exchange.buy(tok, q->sym, q->volume);
        }
        break;
    case SELL:
        {
            client_buysell_body_t *q = (client_buysell_body_t *)(body);
            _exchange.sell(tok, q->sym, q->volume);
        }
        break;
    case QUOTE:
        // cast body
        {
            bid b; 
            offer o;
            client_quote_body_t *q = (client_quote_body_t *)(body);
            if (!_exchange.get_quote(q->sym, &b, &o)) {
                // no quote possible
                message_from_server msg;
                string err = "Couldn't quote ";
                err += q->sym;
                msg.encode_body(err);
                deliver(msg);
            } else {
                // send quote back
                message_from_server msg;
                msg.encode_body(b, o);
                deliver(msg);
            }
        }
        return 1;
    case EXIT:
        return 0;
    default:
        throw "Unknown type";
    };
    
    // must work (fix later)
    bid_offer bo = static_cast<bid_offer>(body);
    _exchange.check_matches(bo.sym);
    
    return 1;
}

void client_session::do_write()
{
    auto self(shared_from_this());
    boost::asio::async_write(_socket,
        boost::asio::buffer(_write_msgs.front().data(),
            _write_msgs.front().length()),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                _write_msgs.pop_front();
                if (!_write_msgs.empty())
                {
                    do_write();
                }
            }
            else
            {
                terminate();
            }
        });
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
    auto cli = all_clients.find(session->get_hash());
    if (cli == all_clients.end()) {
        // client doesn't exist
        all_clients[hash] = new Client(session);
        cout << "New client (" << all_clients[hash]->get_token() << ") has connected" << endl;
    } else {
        // client already exists
        all_clients[hash]->_session = session;
        cout << "Welcome back (" << all_clients[hash]->get_token() << ")" << endl;
    }
    _mtx.unlock();
    return all_clients[hash];
}

void Client::disconnect(client_sess_ptr session) {
    all_clients.erase(session->get_hash());
}


void Client::deliver(const message_from_server& msg) {
    _session->deliver(msg);
}

// -----------------exchange--------------------------

exchange::exchange() {}

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

void exchange::buy(token_t tok, symbol_t sym, int volume) {
    auto &q = _offers[sym];
    
    q.lock();
        
    while (!q.empty() && volume > 0) {
        offer o = q.top();
        
        // logic of removing from volume+bid/offer if necessary        
        if (volume >= o.volume) {
            // left over volume, so remove
            volume -= o.volume;
            match(tok, o.token, sym, o.value, o.volume);
            q.pop();
        } else {
            o.volume -= volume;
            match(tok, o.token, sym, o.value, volume);
            q.pop();
            q.push(o);
            break;
        }
    }
    q.unlock();
}

void exchange::sell(token_t tok, symbol_t sym, int volume) {
    auto &q = _bids[sym];
    
    q.lock();
        
    while (!q.empty() && volume > 0) {
        bid b = q.top();
        
        // logic of removing from volume+bid/offer if necessary        
        if (volume >= b.volume) {
            // left over volume, so remove
            volume -= b.volume;
            match(b.token, tok, sym, b.value, b.volume);
            q.pop();
        } else {
            b.volume -= volume;
            match(b.token, tok, sym, b.value, volume);
            q.pop();
            q.push(b);
            break;
        }
    }
    q.unlock();
}

bool exchange::get_quote(symbol_t sym, bid *b, offer *o) {
    auto &bid_q = _bids[sym];
    auto &offer_q = _offers[sym];
    
    bid_q.lock();
    offer_q.lock();
    
    if (bid_q.empty() || offer_q.empty()) {
        offer_q.unlock();
        bid_q.unlock();
        return false;
    }
    
    *b = bid_q.top();
    *o = offer_q.top();
    
    offer_q.unlock();
    bid_q.unlock();
    return true;
}

void exchange::check_matches(symbol_t sym) {
    // try to match bid+offer
    auto &bid_q = _bids[sym];
    auto &offer_q = _offers[sym];
    
    bid_q.lock();
    offer_q.lock();
        
    while (!bid_q.empty() && !offer_q.empty()) {
        bid matched_bid = bid_q.top();
        offer matched_offer = offer_q.top();
        
        if (matched_bid < matched_offer)
            break;
        
        // logic of removing from volume+bid/offer if necessary
        match(matched_bid.token, matched_offer.token, 
            sym, matched_offer.value, 
            matched_bid.volume > matched_offer.volume 
                ? matched_offer.volume 
                : matched_bid.volume);
        
        if (matched_bid.volume > matched_offer.volume) {
            matched_bid.volume -= matched_offer.volume;
            bid_q.pop();
            bid_q.push(matched_bid);
            
            offer_q.pop();
        } else if (matched_bid.volume == matched_offer.volume) {
            bid_q.pop();
            offer_q.pop();
        } else {
            matched_offer.volume -= matched_bid.volume;
            offer_q.pop();
            offer_q.push(matched_offer);
            
            bid_q.pop();
        }
    }
    offer_q.unlock();
    bid_q.unlock();
}

void exchange::notify_client_of_match(token_t tok, symbol_t sym, double price, int volume, bool is_buyer) {
    // notify both clients that it was sold at offer
    auto client = Client::all_clients[tok];
    
    message_from_server msg;
    
    // symbol_t sym, double price, int volume, bool buyer
    msg.encode_body(sym, price, volume, is_buyer);
    
    client->deliver(msg);
}

void exchange::match(token_t buyer_tok, token_t seller_tok, symbol_t sym, double price, int volume) {
    cout << "Matched buyer " << buyer_tok << " and seller " << seller_tok 
        << " for " << volume << " " << sym << " @ $" << price << endl;
    // notify both clients that it was sold at offer
    notify_client_of_match(buyer_tok, sym, price, volume, true);
    notify_client_of_match(seller_tok, sym, price, volume, false);
}

void exchange::join(Client *client) {
    // _clients.insert(client);
}