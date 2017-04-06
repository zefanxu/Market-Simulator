#include "tcp_server.h"

TCPServer::TCPServer(){
  _endpoint = nullptr;
  _acceptor = nullptr;
  _socket = nullptr;
  market = nullptr;
  alive = false;
  last_exec = clock();
}

TCPServer::TCPServer(unsigned int port){
  if (port <= 0 or port > 65535)
    throw invalid_argument("invalid port number");
  _endpoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
  _acceptor = new asio::ip::tcp::acceptor(io_service, *_endpoint);
  _socket = new asio::ip::tcp::socket(io_service);

  last_exec = clock();
  alive = false;
}

TCPServer::~TCPServer(){
  string text = "Server shutdown";
  l.write(text);
  if (_socket){
    delete _endpoint;
    delete _acceptor;
    delete _socket;
  }
  if (market)
    delete market;
}

void TCPServer::accept(){
  if (!_socket)
    throw runtime_error("Invalid socket");
  try{
    _acceptor->accept(*_socket);
  }
  catch (std::exception& e){
    l.write(e.what());
    throw e;
  }
  market->setLogger(&l);
  _socket->non_blocking(true);
  alive = true;
}

bool TCPServer::isAlive(){
  return alive;
}

bool TCPServer::should_execute(){
  clock_t curr_time = clock();
  double interval = (double)1/(double)MAX_EXEC_PER_SECOND;
  double time_passed = (double) (clock() - last_exec) / CLOCKS_PER_SEC;
  if (time_passed >= interval){
    last_exec = curr_time;
    return true;
  }
  return false;
}

void TCPServer::process(char * buf, size_t len){
  if (!market) throw runtime_error("uninitialized server");
  if (len)
    market->handle_packet(buf, len);
  if (should_execute())
    market->market_logic();
}

void SoupBinTCPServer::send(){
  if (!alive) return;
  boost::system::error_code ec;
  for (const auto & msg : market->pending_out_messages){
    l.write("SEND: "+ouch::inbound_to_string(reinterpret_cast<const ouch::MsgHeader*>(&msg[0])));
    asio::write(*_socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
  }
  market->pending_out_messages.clear();
}

SoupBinTCPServer::SoupBinTCPServer(){
  read_pos = nullptr;
  market = new ouch_session();
}

SoupBinTCPServer::SoupBinTCPServer(unsigned int port):TCPServer(port){
  read_pos = nullptr;
  market = new ouch_session();
}

int SoupBinTCPServer::read(char* & outbuf){
  boost::system::error_code ec;
  if (read_pos)
    read_pos += packet_len;
  if (!read_pos or read_pos >= buf.c_array() + read_len){
    read_len = _socket->read_some(asio::buffer(buf), ec);
    read_pos = buf.c_array();
  }
  if (ec == asio::error::would_block){
    outbuf = nullptr;
    return 0;
  }
  else if (ec == asio::error::eof){
    string text = "Connection Closed";
    l.write(text);
    alive = false;
    outbuf = nullptr;
    return 0;
  }
  l.write("RECV: " + ouch::outbound_to_string(reinterpret_cast<const ouch::MsgHeader*>(read_pos)));
  packet_len = big_to_native((reinterpret_cast<const ouch::MsgHeader*>(read_pos))->length) + 2;
  outbuf = read_pos;
  return packet_len;
}

void BOEServer::send(){
  if (!alive) return;
  boost::system::error_code ec;
  for (const auto & msg : market->pending_out_messages){
    l.write("SEND: "+boe::to_string(reinterpret_cast<const boe::MsgHeader*>(&msg[0])));
    asio::write(*_socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
  }
  market->pending_out_messages.clear();
}

BOEServer::BOEServer(){
  read_pos = nullptr;
  market = new boe_session();
}

BOEServer::BOEServer(unsigned int port):TCPServer(port){
  read_pos = nullptr;
  market = new boe_session();
}

int BOEServer::read(char* & outbuf){
  boost::system::error_code ec;
  if (read_pos)
    read_pos += packet_len;
  if (!read_pos or read_pos >= buf.c_array() + read_len){
    read_len = _socket->read_some(asio::buffer(buf), ec);
    read_pos = buf.c_array();
  }
  if (ec == asio::error::would_block){
    outbuf = nullptr;
    return 0;
  }
  else if (ec == asio::error::eof){
    string text = "Connection Closed";
    l.write(text);
    alive = false;
    outbuf = nullptr;
    return 0;
  }
  packet_len = (reinterpret_cast<const boe::MsgHeader*>(read_pos))->length + 2;
  l.write("RECV: " + boe::to_string(reinterpret_cast<const boe::MsgHeader*>(read_pos)));
  outbuf = read_pos;
  return packet_len;
}
