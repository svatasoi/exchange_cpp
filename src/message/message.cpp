#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
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
    _body_length = std::atoi(header);
    if (_body_length > max_body_length)
    {
      _body_length = 0;
      return false;
    }
    return true;
}

void message_from_client::encode_header() {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(_body_length));
    std::memcpy(_data, header, header_length);
}

// --------message from server->client--------

bool message_from_server::decode_header() {
    return false;
}

void message_from_server::encode_header() {
    
}