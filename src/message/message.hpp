#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

enum client_message_type { BID=0, OFFER=1 };
struct client_header_t {
    client_message_type type;
    int body_length;
};

struct client_body_t {
    char sym[4];
    double price;
    int volume;
};

class message
{
public:
    static const int header_length = sizeof(client_header_t);
    // enum { header_length = sizeof(header_t) };
    enum { max_body_length = 512 };

    message();

    const char* data() const;
    char* data();
    std::size_t length() const;
    const char* body() const;
    char* body();
    std::size_t body_length() const;
    void body_length(std::size_t new_length);

    virtual bool decode_header() = 0;
    virtual void encode_header() = 0;

protected:
    char _data[header_length + max_body_length];
    std::size_t _body_length;
};
    
class message_from_client : public message
{
public:
    virtual bool decode_header();
    virtual void encode_header();
    
    int encode_body(std::string input);
private:
    client_message_type _message_type;
};

class message_from_server : public message
{
public:
    virtual bool decode_header();
    virtual void encode_header();
};