#include <cstdlib>
#include <deque>
#include <memory>
#include <iostream>
#include <list>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <boost/asio.hpp>
#include "server/exchange.cpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

class exchange_server
{
public:
  exchange_server(boost::asio::io_service& io_service,
      const tcp::endpoint& endpoint,
      exchange& exchge)
    : _acceptor(io_service, endpoint),
      _socket(io_service),
      _exchange(exchge)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    _acceptor.async_accept(_socket,
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<client_session>(std::move(_socket), _exchange)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor _acceptor;
  tcp::socket _socket;
  exchange _exchange;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: exchange_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    exchange exchge;
    tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
    exchange_server server(io_service, endpoint, exchge);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}