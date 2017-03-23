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
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>

#include <evt_util.h>
#include "ouch_messages.h"
#include "boe_messages.h"

using namespace std;
using namespace evt;

using boost::endian::big_to_native;
using boost::endian::native_to_big;

namespace evt{
namespace ouch{
const std::string inbound_to_string(const MsgHeader* m);
const std::string outbound_to_string(const MsgHeader* m);
}
namespace boe{
const std::string to_string(const MsgHeader* msg);
}
}

class logger{
public:
  logger();
  ~logger();
  void write(string text);
  bool also_print;
private:
  ofstream file;
  string get_time_string();
  string get_datetime_string();
};


#endif
