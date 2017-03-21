#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "evtsim_util.h"
#include "evtsim_messages.h"
#include "ouch_session.h"

using namespace std;
using namespace evt;

namespace asio = boost::asio;
typedef vector<char> message;

class TCPServer{
public:
  TCPServer();
  TCPServer(unsigned int port);
  ~TCPServer();
  void accept();
  bool isAlive();
  virtual int read(char* & buf);
  virtual void send(vector<message> & msgs);

protected:
  unsigned int port;
  bool alive;
  asio::io_service io_service;
  asio::ip::tcp::endpoint _endpoint;
  asio::ip::tcp::acceptor _acceptor;
  asio::ip::tcp::socket _socket;
  logger l;
};

class SoupBinTCPServer: public TCPServer{
public:
  SoupBinTCPServer();
  SoupBinTCPServer(unsigned int port);
  virtual int read(char* & buf);
  virtual void send(vector<message> & msgs);

private:
  unsigned int packet_len, read_len;
  char * read_pos;
  boost::array<char, 4096> buf;
};
