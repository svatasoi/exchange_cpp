#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "server/bid_offer.hpp"
#include "message.hpp"

// -------generic message---------

message::message() : _body_length(0) {}

const char* message::data() const
{
    return _data;
}

char* message::data()
{
    return _data;
}

std::size_t message::length() const
{
    return header_length + _body_length;
}

const char* message::body() const
{
    return _data + header_length;
}

char* message::body()
{
    return _data + header_length;
}

std::size_t message::body_length() const
{
    return _body_length;
}

void message::body_length(std::size_t new_length)
{
    _body_length = new_length;
    if (_body_length > max_body_length)
        _body_length = max_body_length;
}

// --------message from client->server--------

bool message_from_client::decode_header() {
    char header[header_length + 1] = "";
    std::strncat(header, _data, header_length);
    
    stringstream head{string(header)};
    
    int mess_type;
    head >> mess_type >> _body_length;
    
    _message_type = static_cast<client_message_type>(mess_type);
    if (_body_length > max_body_length)
    {
      _body_length = 0;
      return false;
    }
    return true;
}

void message_from_client::encode_header() {
    char header[header_length + 1] = "";
    std::sprintf(header, "%d %d", 
        static_cast<int>(_message_type), 
        static_cast<int>(_body_length));
    std::memcpy(_data, header, header_length);
}

#define MAX_BUFFER_LENGTH 512
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
    } else if (!buffer.compare("exit")) {
        return 0; // done
    } else {
        return -1;
    }
    
    bid_offer bo;
    input >> bo.sym >> bo.value >> bo.volume;
    // if ((res = (input >> bo.sym >> bo.value >> bo.volume)) < 0) {
    //     return res;
    // }
    
    // 4 is SYMBOL_LEN
    res = std::sprintf(body(), "%4s %lf %d", 
        bo.sym.c_str(), bo.value, bo.volume);
    if (res < 0) {
        return res;
    }
    _body_length = res;
    
    encode_header();
    return _body_length;
}

// --------message from server->client--------

bool message_from_server::decode_header() {
    return false;
}

void message_from_server::encode_header() {
    
}