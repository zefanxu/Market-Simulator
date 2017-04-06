#include "tcp_server.h"

TCPServer::TCPServer(unsigned int port, asio::io_service* io_service){
  if (port <= 0 or port > 65535)
    throw invalid_argument("Invalid port number");
  _io_service = io_service;
  _endpoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
  _acceptor = new asio::ip::tcp::acceptor(*_io_service, *_endpoint);
  _socket = new asio::ip::tcp::socket(*_io_service);
  _timer = new asio::deadline_timer(*_io_service, boost::posix_time::milliseconds(1000/MAX_EXEC_PER_SECOND));
  if (!_socket)
    throw runtime_error("Invalid socket");
  l.write("[TCP]Accepting Connection on port " + to_string(port));
  _acceptor->async_accept(*_socket, boost::bind(&TCPServer::accept, this, asio::placeholders::error));
}

TCPServer::~TCPServer(){
  l.write("[TCP]Server shutdown");
  if (_socket){
    delete _endpoint;
    delete _acceptor;
    delete _socket;
  }
  if (market)
    delete market;
  if (_timer)
    delete _timer;
}

void TCPServer::accept(const boost::system::error_code& error){
  if (error) return;
  l.write("[TCP]Connection established");
  market->setLogger(&l);
  _socket->async_read_some(asio::buffer(buf),
                boost::bind(&TCPServer::read, this,
                asio::placeholders::error, asio::placeholders::bytes_transferred));
  _timer->async_wait(boost::bind(&TCPServer::process, this, asio::placeholders::error));
}

void TCPServer::process(const boost::system::error_code& error){
  if (error) return;
  if (!market) throw runtime_error("Uninitialized server");
  market->market_logic();
  send();
  _timer->expires_at(_timer->expires_at() + boost::posix_time::milliseconds(1000/MAX_EXEC_PER_SECOND));
  _timer->async_wait(boost::bind(&TCPServer::process, this, asio::placeholders::error));
}

void TCPServer::reconnect(){
  l.write("[TCP]Attempt to reconnect");
  if (_socket)
    delete _socket;
  _socket = new asio::ip::tcp::socket(*_io_service);
  _acceptor->async_accept(*_socket, boost::bind(&TCPServer::accept, this, asio::placeholders::error));
}

void SoupBinTCPServer::send(){
  boost::system::error_code ec;
  for (const auto & msg : market->pending_out_messages){
    l.write("[OUCH]SEND: "+ouch::inbound_to_string(reinterpret_cast<const ouch::MsgHeader*>(&msg[0])));
    asio::write(*_socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
  }
  market->pending_out_messages.clear();
}

SoupBinTCPServer::SoupBinTCPServer(unsigned int port, asio::io_service* io_service):TCPServer(port, io_service){
  market = new ouch_session();
}

void SoupBinTCPServer::read(boost::system::error_code ec, size_t bytes_received){
  char * read_pos = buf.c_array();
  size_t packet_len;
  if (ec == asio::error::eof){
    l.write("[OUCH]Connection Closed");
    _timer->cancel();
    reconnect();
    return;
  }
  while (read_pos < buf.c_array() + bytes_received){
    l.write("[OUCH]RECV: " + ouch::outbound_to_string(reinterpret_cast<const ouch::MsgHeader*>(read_pos)));
    packet_len = big_to_native((reinterpret_cast<const ouch::MsgHeader*>(read_pos))->length) + 2;
    if (packet_len)
      market->handle_packet(read_pos, packet_len);
    read_pos += packet_len;
  }
  send();
  _socket->async_read_some(asio::buffer(buf),
                boost::bind(&SoupBinTCPServer::read, this,
                asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void BOEServer::send(){
  boost::system::error_code ec;
  for (const auto & msg : market->pending_out_messages){
    l.write("[BOE]SEND: "+boe::to_string(reinterpret_cast<const boe::MsgHeader*>(&msg[0])));
    asio::write(*_socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
  }
  market->pending_out_messages.clear();
}

BOEServer::BOEServer(unsigned int port, asio::io_service* io_service):TCPServer(port, io_service){
  market = new boe_session();
}

void BOEServer::read(boost::system::error_code ec, size_t bytes_received){
  char * read_pos = buf.c_array();
  size_t packet_len;
  if (ec == asio::error::eof){
    l.write("[BOE]Connection Closed");
    _timer->cancel();
    reconnect();
    return;
  }
  while (read_pos < buf.c_array() + bytes_received){
    l.write("[BOE]RECV: " + boe::to_string(reinterpret_cast<const boe::MsgHeader*>(read_pos)));
    packet_len = (reinterpret_cast<const boe::MsgHeader*>(read_pos))->length + 2;
    if (packet_len)
      market->handle_packet(read_pos, packet_len);
    read_pos += packet_len;
  }
  send();
  _socket->async_read_some(asio::buffer(buf),
                boost::bind(&BOEServer::read, this,
                asio::placeholders::error, asio::placeholders::bytes_transferred));
}
