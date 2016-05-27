#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "server/bid_offer.hpp"

// ---------------client message types---------------

enum client_message_type { BID=0, OFFER=1, BUY=2, SELL=3, QUOTE=4, EXIT=5 };
struct client_header_t {
    client_message_type type;
    int body_length;
};

struct client_buysell_body_t {
    char sym[SYMBOL_LEN];
    char nbyte = 0;
    int volume;
};

struct client_bidoffer_body_t {
    char sym[SYMBOL_LEN];
    char nbyte = 0;
    double price;
    int volume;
};

struct client_quote_body_t {
    char sym[SYMBOL_LEN];
    char nbyte = 0;
};

// ---------------server message types---------------

enum server_message_type { MATCH=0, QUOTE_RESPONSE=1, SERVER_ERROR=2 };
struct server_header_t {
    server_message_type type;
    int body_length;
};

struct server_match_body_t {
    char sym[SYMBOL_LEN];
    char nbyte = 0;
    double price;
    int volume;
    bool buyer; // true for if receiver is buyer, false for seller
};

struct server_quote_body_t {
    char sym[SYMBOL_LEN];
    char nbyte = 0;
    double bid;
    int bid_vol;
    double offer;
    int offer_vol;
};

#define MAX_MSG_SIZE 128
struct server_err_body_t {
    char msg[MAX_MSG_SIZE];
};

// ---------------message classes---------------

template<class HT, class MT>
class message
{
public:
    static const int header_length = sizeof(HT);
    static const int max_body_length = 512;

    message();

    const char* data() const;
    char* data();
    std::size_t length() const;
    const char* body() const;
    char* body();
    std::size_t body_length() const;
    void body_length(std::size_t new_length);

    bool decode_header();
    void encode_header();
protected:
    char _data[header_length + max_body_length];
    std::size_t _body_length;
    MT _message_type;
};

// -------generic message---------

template<class HT, class MT>
message<HT,MT>::message() : _body_length(0) {}

template<class HT, class MT>
const char* message<HT,MT>::data() const
{
    return _data;
}

template<class HT, class MT>
char* message<HT,MT>::data()
{
    return _data;
}

template<class HT, class MT>
std::size_t message<HT,MT>::length() const
{
    return message<HT,MT>::header_length + _body_length;
}

template<class HT, class MT>
const char* message<HT,MT>::body() const
{
    return _data + message<HT,MT>::header_length;
}

template<class HT, class MT>
char* message<HT,MT>::body()
{
    return _data + message<HT,MT>::header_length;
}

template<class HT, class MT>
std::size_t message<HT,MT>::body_length() const
{
    return _body_length;
}

template<class HT, class MT>
void message<HT,MT>::body_length(std::size_t new_length)
{
    _body_length = new_length;
    if (_body_length > max_body_length)
        _body_length = max_body_length;
}

template<class HT, class MT>
bool message<HT,MT>::decode_header() {
    HT *header = (HT *)_data;
    _message_type = header->type;
    _body_length = header->body_length;
    
    if (_body_length > max_body_length)
    {
      _body_length = 0;
      return false;
    }
    return true;
}

template<class HT, class MT>
void message<HT,MT>::encode_header() {
    HT header{};
    header.type = _message_type;
    header.body_length = _body_length;
    std::memcpy(_data, &header, sizeof(HT));
}

// -------------------messages from client-------------------

class message_from_client : public message<client_header_t, client_message_type>
{
public:
    int encode_body(std::string input);
    bool is_exit_msg();
};

// -------------------messages from server-------------------

class message_from_server : public message<server_header_t, server_message_type>
{
public:
    int encode_body(symbol_t sym, double price, int volume, bool buyer);
    int encode_body(bid &b, offer &o);
    int encode_body(string &msg);
};