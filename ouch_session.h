#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "evtsim_util.h"

namespace ouch_state{
  not_logged_in = 'N';
  logged_in = 'L';
}

class ouch_session{
public:
  ouch_session();
  init();
  string parse_packet(char * packet, size_t len);
private:
  string handle_login_request(MsgHeader * packet, size_t len);
  string handle_logout_request(MsgHeader * packet, size_t len);
  bool login(packet);
  unordered_map<string, vector<unsigned int>> messages;
  vector<string> session_id;
  char state;
};
