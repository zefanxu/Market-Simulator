#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "evtsim_util.h"
#include "ouch_messages.h"
#include "session.h"

//maxinum number of execution per second
#define MAX_EXEC_PER_SECOND 3

using namespace std;
using namespace evt;

namespace asio = boost::asio;
typedef vector<char> message;

class TCPServer{
public:
  TCPServer(unsigned int port, asio::io_service* io_service, evtsim::Logger * logger);
  virtual ~TCPServer();
  void accept(const boost::system::error_code& error);
  virtual void read(boost::system::error_code ec, size_t bytes_received)=0;
  virtual void send()=0;
  virtual void process(const boost::system::error_code& error);
  virtual void reconnect();

protected:
  session* market;
  asio::deadline_timer* _timer;
  asio::io_service* _io_service;
  asio::ip::tcp::endpoint* _endpoint;
  asio::ip::tcp::acceptor* _acceptor;
  asio::ip::tcp::socket* _socket;
  evtsim::Logger* l;
  boost::array<char, 4096> buf;
};

class SoupBinTCPServer: public TCPServer{
public:
  SoupBinTCPServer(unsigned int port, asio::io_service* io_service);
  virtual void read(boost::system::error_code ec, size_t bytes_received);
  virtual void send();
};

class BOEServer: public TCPServer{
public:
  BOEServer(unsigned int port, asio::io_service* io_service);
  virtual void read(boost::system::error_code ec, size_t bytes_received);
  virtual void send();
};
