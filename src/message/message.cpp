#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "server/bid_offer.hpp"
#include "message.hpp"

// --------message from client->server--------

int message_from_client::encode_body(string in) {
    string buffer;
    // input should be <type> <symbol> <price> <amount>
    stringstream input(in);
    int res;
    input >> buffer;
    // if ((res = (input >> buffer)) < 0) {
    //     return res;
    // }
    
    if (!buffer.compare("bid") || !buffer.compare("b")) {
        _message_type = BID;
    } else if (!buffer.compare("offer") || !buffer.compare("o")) {
        _message_type = OFFER;
    } else if (!buffer.compare("quote") || !buffer.compare("q")) {
        symbol_t sym;
        input >> sym;
        std::memcpy(body(), sym.c_str(), SYMBOL_LEN);
        _body_length = sizeof(client_quote_body_t);
        _message_type = QUOTE;
        encode_header();
        return _body_length;
    } else if (!buffer.compare("exit")) {
        _message_type = EXIT;
        _body_length = 0;
        return 0; // done
    } else {
        return -1;
    }
    
    bid_offer bo;
    input >> bo.sym >> bo.value >> bo.volume;
         
    client_bidoffer_body_t bod{};
    std::memcpy(bod.sym, bo.sym.c_str(), SYMBOL_LEN); 
    bod.price = bo.value;
    bod.volume= bo.volume;
    std::memcpy(body(), &bod, sizeof(bod));
    _body_length = sizeof(bod);
    
    encode_header();
    return _body_length;
}

bool message_from_client::is_exit_msg() {
    return _message_type == EXIT;
}

// --------message from server->client--------

int message_from_server::encode_body(symbol_t sym, double price, int volume, bool buyer) {
    server_match_body_t bod{};
    
    std::memcpy(bod.sym, sym.c_str(), SYMBOL_LEN); 
    bod.price = price;
    bod.volume= volume;
    bod.buyer = buyer;
    std::memcpy(body(), &bod, sizeof(bod));
    
    _message_type = MATCH;
    _body_length = sizeof(bod);
    
    encode_header();
    return _body_length;
}

int message_from_server::encode_body(bid &b, offer &o) {
    server_quote_body_t bod{};
    
    std::memcpy(bod.sym, b.sym.c_str(), SYMBOL_LEN); 
    bod.bid = b.value;
    bod.bid_vol = b.volume;
    bod.offer = o.value;
    bod.offer_vol = o.volume;
    std::memcpy(body(), &bod, sizeof(bod));
    
    _message_type = QUOTE_RESPONSE;
    _body_length = sizeof(bod);
    
    encode_header();
    return _body_length;
}