#ifndef _OUCH_SESSION_H
#define _OUCH_SESSION_H
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <time.h>
#include "evtsim_util.h"
#include "evtsim_messages.h"

using namespace evt::ouch;
namespace ouch_state{
  constexpr char not_logged_in = 'N';
  constexpr char logged_in = 'L';
  constexpr char losing_heartbeat = '1';
}

class ouch_session{
public:
  void init();
  ouch_session();
  string parse_packet(char * packet, size_t len);
private:
  string handle_login_request(MsgHeader * packet, size_t len);
  string handle_logout_request(MsgHeader * packet, size_t len);
  string handle_client_heartbeat(MsgHeader * packet, size_t len);
  string heartbeat();
  bool login(LoginRequest * req);
  unordered_map<string, vector<unsigned int>> messages;
  vector<string> session_id;
  clock_t last_send_heartbeat;
  clock_t last_recv_heartbeat;
  char state;
};

#endif
