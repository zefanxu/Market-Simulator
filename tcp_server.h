#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "evtsim_util.h"
#include "ouch_messages.h"
#include "session.h"

using namespace std;
using namespace evt;

namespace asio = boost::asio;
typedef vector<char> message;

class TCPServer{
public:
  TCPServer();
  TCPServer(unsigned int port);
  virtual ~TCPServer();
  void accept();
  bool isAlive();
  virtual int read(char* & buf)=0;
  virtual void send()=0;
  virtual void process(char * buf, size_t len);

protected:
  session* market;
  bool alive;
  asio::io_service io_service;
  asio::ip::tcp::endpoint* _endpoint;
  asio::ip::tcp::acceptor* _acceptor;
  asio::ip::tcp::socket* _socket;
  logger l;
};

class SoupBinTCPServer: public TCPServer{
public:
  SoupBinTCPServer();
  SoupBinTCPServer(unsigned int port);
  virtual int read(char* & outbuf);
  virtual void send();

private:
  unsigned int packet_len, read_len;
  char * read_pos;
  boost::array<char, 4096> buf;
};

class BOEServer: public TCPServer{
public:
  BOEServer();
  BOEServer(unsigned int port);
  virtual int read(char* & outbuf);
  virtual void send();

private:
  unsigned int packet_len, read_len;
  char * read_pos;
  boost::array<char, 4096> buf;
};
