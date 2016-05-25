#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <string>

#include <chrono>

#include <boost/asio.hpp>
#include "message/message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<message_from_client> message_queue;

class exchange_client
{
public:
  exchange_client(boost::asio::io_service& io_service,
      tcp::resolver::iterator endpoint_iterator)
    : _io_service(io_service),
      _socket(io_service)
  {
    do_connect(endpoint_iterator);
  }

  void write(const message_from_client& msg)
  {
    _io_service.post(
        [this, msg]()
        {
          bool write_in_progress = !_write_msgs.empty();
          _write_msgs.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }

  void close()
  {
    _io_service.post([this]() { _socket.close(); });
  }

private:
  void do_connect(tcp::resolver::iterator endpoint_iterator)
  {
    boost::asio::async_connect(_socket, endpoint_iterator,
        [this](boost::system::error_code ec, tcp::resolver::iterator)
        {
          if (!ec)
          {
            do_read_header();
          }
        });
  }

  void do_read_header()
  {
    boost::asio::async_read(_socket,
        boost::asio::buffer(_read_msg.data(), message_from_server::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec && _read_msg.decode_header())
          {
              // process message from server
            do_read_body();
          }
          else
          {
            _socket.close();
          }
        });
  }

  void do_read_body()
  {
    boost::asio::async_read(_socket,
        boost::asio::buffer(_read_msg.body(), _read_msg.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            std::cout.write(_read_msg.body(), _read_msg.body_length());
            std::cout << "\n";
            do_read_header();
          }
          else
          {
            _socket.close();
          }
        });
  }

  void do_write()
  {
    boost::asio::async_write(_socket,
        boost::asio::buffer(_write_msgs.front().data(),
          _write_msgs.front().length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
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
            _socket.close();
          }
        });
  }

private:
  boost::asio::io_service& _io_service;
  tcp::socket _socket;
  message_from_server _read_msg;
  message_queue _write_msgs;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
    exchange_client c(io_service, endpoint_iterator);

    std::thread t([&io_service](){ io_service.run(); });

    std::string input;
    while (std::getline(std::cin, input))
    {
      message_from_client msg;
      int err = msg.encode_body(input);
      if (err < 0) {
          std::cout << "Invalid message" << std::endl;
          continue;
      } else if (err == 0) {
          std::cout << "Exiting on: " << input << std::endl;
          break;
      }
      c.write(msg);
    }
    
    // sleep
    // mehh for script to work?
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 

    c.close();
    t.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}