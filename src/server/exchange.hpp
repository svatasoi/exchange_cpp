#pragma once

#include <set>
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <boost/asio.hpp>
#include "linked_list/linked_list.cpp"
#include "concurrent_map/concurrent_map.cpp"

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
      _exchange(exc)
  {
  }

  size_t get_hash() {
    tcp::endpoint endpoint = _socket.remote_endpoint();
    std::ostringstream stream;
    stream << endpoint;
    std::hash<std::string> hasher;
    return hasher(stream.str());
  }

  void start();
/*
  void deliver(const chat_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
      do_write();
    }
  }

private:
  void do_read_header()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](boost::system::error_code ec, std::size_t)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](boost::system::error_code ec, std::size_t)
        {
          if (!ec)
          {
            room_.deliver(read_msg_);
            do_read_header();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this, self](boost::system::error_code ec, std::size_t)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }
*/
  tcp::socket _socket;
  exchange& _exchange;
//   chat_message read_msg_;
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


void client_session::start() {
    Client::connect(shared_from_this());
    // do_read_header();
}