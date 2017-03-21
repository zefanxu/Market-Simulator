#include "tcp_server.h"

TCPServer::TCPServer(){
  _endpoint = nullptr;
  _acceptor = nullptr;
  _socket = nullptr;
  port = 0;
  alive = false;
}

TCPServer::TCPServer(unsigned int port){
  _endpoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
  _acceptor = new asio::ip::tcp::acceptor(io_service, _endpoint);
  _socket = new asio::ip::tcp::socket(io_service);
  alive = false;
}

TCPServer::~TCPServer(){
  string text = "Conenction Closed";
  l.write(text);
  if (alive){
    delete _endpoint;
    delete _acceptor;
    delete _socket;
  }
}

void TCPServer::accept(){
  if (port <= 0 or port > 65535)
    throw invalid_argument("invalid port number");
  try{
    _acceptor->accept(*_socket);
  }
  catch (std::exception& e){
    l.write(e.what());
    throw e;
  }
  alive = true;
}

bool TCPServer::isAlive(){
  return alive;
}

void SoupBinTCPServer::send(vector<message> & msgs){
  if (!alive) return;
  boost::system::error_code ec;
  for (const auto & msg : msgs){
    l.write("SEND: "+inbound_to_string(reinterpret_cast<const MsgHeader*>(&msg[0])));
    asio::write(*_socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
  }
  msgs.clear();
}

SoupBinTCPServer::SoupBinTCPServer(){
  read_pos = nullptr;
}

SoupBinTCPServer::SoupBinTCPServer(unsigned int port):TCPServer(port){
  read_pos = nullptr;
}

int SoupBinTCPServer::read(char* & buf){
  boost::system::error_code ec;
  if (read_pos)
    read_pos += packet_len;
  if (!read_pos or read_pos >= buf.c_array() + read_len){
    read_len = _socket->read_some(asio::buffer(buf), ec);
    read_pos = buf.c_array();
  }
  if (ec == asio::error::would_block){
    buf = nullptr;
    return 0;
  }
  else if (ec == asio::error::eof){
    alive = false;
    buf = nullptr;
    return 0;
  }
  l.write("RECV: " + outbound_to_string(reinterpret_cast<const MsgHeader*>(read_pos)));
  packet_len = big_to_native((reinterpret_cast<const MsgHeader*>(read_pos))->length) + 2;
  buf = read_pos;
  return packet_len;
}
