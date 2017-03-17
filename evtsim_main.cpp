#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

#include <evt_server.h>
#include <evt_util.h>
#include "evtsim_util.h"
#include "evtsim_messages.h"
#include "ouch_session.h"

using namespace std;
using namespace evt;
namespace bpo = boost::program_options;
namespace asio = boost::asio;

void
signal_handler(int signum) {
}

const char*
get_evtsim_version() {
#ifdef EVTSIM_VERSION
  return EVTSIM_VERSION;
#else
  return "undef";
#endif
}

int
main(int argc, char** argv) {
  srand(time(NULL));
  bpo::options_description opts("Available options");
  opts.add_options()
    ("help,h", "produce help message")
    ("config,c", bpo::value<string>(), "specify configuration file")
    ("version", "show version")
    ("port,p", bpo::value<int>(), "simulator server listen port number")
    ;

  bpo::variables_map vm;
  try {
    bpo::store(bpo::parse_command_line(argc, argv, opts), vm);
  } catch(bpo::error& e) {
    cout << "Options error: " << e.what() << endl;
    return 4;
  }

  bpo::notify(vm);

  if(vm.count("version")) {
    cout << get_evtsim_version() << endl;
    return 0;
  }

  if(vm.count("help")) {
    cout << opts << endl;
    return 1;
  }

  // if(!vm.count("config")) {
  //   cout << opts << endl;
  //   return 2;
  // }
  if (!vm.count("port")){
    cout << "missing port number" << endl;
    cout << opts << endl;
    return 2;
  }

  int port_num = vm["port"].as<int>();
  ouch_session s;
  boost::array<char, 128> buf;
  asio::io_service io_service;
  asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port_num);
  boost::system::error_code ec;
  asio::ip::tcp::acceptor acceptor(io_service, endpoint);
  asio::ip::tcp::socket socket(io_service);
  try{
    acceptor.accept(socket);
  }
  catch (std::exception& e){
    std::cerr << "Exception: " << e.what() <<endl;
    return 2;
  }
  size_t read_len, packet_len;
  char * read_pos;
  while (true){
    s.market_logic();
    read_len = socket.read_some(asio::buffer(buf), ec);
    read_pos = buf.c_array();
    while ((ec != asio::error::would_block) and (read_pos < buf.c_array() + read_len)){
      cout << "RECV: " << outbound_to_string(reinterpret_cast<const MsgHeader*>(read_pos)) << endl;
      packet_len = (reinterpret_cast<const MsgHeader*>(read_pos))->length + 2;
      s.handle_packet(read_pos, packet_len);
      read_pos += packet_len;
    }
    if (ec == asio::error::eof){
      cout << "Connection closed" << endl;
      break;
    }
    for (const auto & msg : s.pending_out_messages){
      cout << "SEND: " << inbound_to_string(reinterpret_cast<const MsgHeader*>(&msg[0])) << endl;
      asio::write(socket, asio::buffer(&msg[0], msg.size()), asio::transfer_all(), ec);
    }
    s.pending_out_messages.clear();
  }
  return 0;
}
