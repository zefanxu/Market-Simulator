#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "evtsim_util.h"
#include "evtsim_messages.h"

namespace ouch_state{
  char not_logged_in = 'N';
  char logged_in = 'L';
}

class ouch_session{
public:
  ouch_session();
  void init();
  string parse_packet(char * packet, size_t len);
private:
  string handle_login_request(MsgHeader * packet, size_t len);
  string handle_logout_request(MsgHeader * packet, size_t len);
  bool login(LoginRequest * req);
  unordered_map<string, vector<unsigned int>> messages;
  vector<string> session_id;
  char state;
};
