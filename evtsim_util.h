#ifndef _EVTSIM_UTIL_H
  #define _EVTSIM_UTIL_H

#include <signal.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>

#include <iostream>
#include <string>
#include <cstring>

#include <evt_server.h>
#include <evt_util.h>
#include "evtsim_messages.h"

using namespace std;
using namespace evt;
using namespace evt::ouch;
using boost::endian::big_to_native;
using boost::endian::native_to_big;

const std::string inbound_to_string(const MsgHeader* m);
const std::string outbound_to_string(const MsgHeader* m);



#endif
