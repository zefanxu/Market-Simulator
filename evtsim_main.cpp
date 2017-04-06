#include <signal.h>
#include <stdlib.h>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <unistd.h>
#include "tcp_server.h"

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
    // ("config,c", bpo::value<string>(), "specify configuration file")
    // ("version", "show version")
    ("ouchport,o", bpo::value<int>(), "ouch server listen port number")
    ("boeport,b", bpo::value<int>(), "boe server listen port number")
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

  vector<unique_ptr<TCPServer>> servers;
  asio::io_service io_service;

  if (!vm.count("ouchport") and !vm.count("boeport")){
    cout << "need at least one port number" << endl;
    cout << opts << endl;
    return 2;
  }
  if (vm.count("ouchport")){
    int ouch_port_num = vm["ouchport"].as<int>();
    servers.push_back(unique_ptr<TCPServer>(new SoupBinTCPServer(ouch_port_num, &io_service)));
  }
  if (vm.count("boeport")){
    int boe_port_num = vm["boeport"].as<int>();
    servers.push_back(unique_ptr<TCPServer>(new BOEServer(boe_port_num, &io_service)));
  }

  io_service.run();

  return 0;
}
