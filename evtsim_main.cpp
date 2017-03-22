#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <unistd.h>
#include <evt_server.h>
#include <evt_util.h>
#include "evtsim_util.h"
#include "evtsim_messages.h"
#include "ouch_session.h"
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

  if (!vm.count("port")){
    cout << "missing port number" << endl;
    cout << opts << endl;
    return 2;
  }
  int port_num = vm["port"].as<int>();

  ouch_session session;
  char * buf;
  TCPServer* s = new SoupBinTCPServer(port_num);
  s->accept();
  while (s->isAlive()){
    size_t len = s->read(buf);
    if (len)
      session.handle_packet(buf, len);
    usleep(100000);
    session.market_logic();
    s->send(session.pending_out_messages);
  }
  delete s;
  return 0;
}
