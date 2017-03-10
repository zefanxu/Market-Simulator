#ifndef _EVTSIM_UTIL_H
  #define _EVTSIM_UTIL_H

#include <signal.h>
#include <stdlib.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <cstring>

#include <evt_server.h>
#include <evt_util.h>
#include "../evts/sessions/ouch/messages.h"

using namespace std;
using namespace evt;

string parse_packet(char * packet, size_t len);
string parse_login_request(char * packet, size_t len);



#endif
