#include <signal.h>
#include <stdlib.h>
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
  try{
    boost::array<char, 128> buf;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port_num);
    boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
    boost::asio::ip::tcp::socket socket(io_service);
    boost::system::error_code ec;
    acceptor.accept(socket);
    size_t len;
    string ret;
    while (true){
      do {
         ret = s.heartbeat();
         if (ret.size()) goto send_packet;
         len = socket.read_some(boost::asio::buffer(buf), ec);
      } while(ec == boost::asio::error::would_block);
      if (ec == boost::asio::error::eof){
        cout << "Connection closed" << endl;
        break;
      }
      cout << "RECV: " << outbound_to_string(reinterpret_cast<const MsgHeader*>(buf.c_array())) << endl;
      ret = s.parse_packet(buf.c_array(), len);
    send_packet:
      if (ret.size()){
        cout << "SEND: " << inbound_to_string(reinterpret_cast<const MsgHeader*>(ret.c_str())) << endl;
        boost::asio::write(socket, boost::asio::buffer(ret), boost::asio::transfer_all(), ec);
      }
    }
  }
  catch (std::exception& e){
    std::cerr << "Exception: " << e.what() <<endl;
  }
  return 0;
}
