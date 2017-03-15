#ifndef _OUCH_SESSION_H
#define _OUCH_SESSION_H
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <time.h>
#include <chrono>
#include "evtsim_util.h"
#include "evtsim_messages.h"
#include "order.h"

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
  vector<char> execute_logic();
  vector<char> parse_packet(char * packet, size_t len);
  vector<char> parse_message(MsgHeader* packet, size_t len);
  vector<char> heartbeat();

private:
  vector<char> handle_login_request(MsgHeader * packet, size_t len);
  vector<char> handle_logout_request(MsgHeader * packet, size_t len);
  vector<char> handle_client_heartbeat(MsgHeader * packet, size_t len);

  vector<char> enterOrder(Ouch_MsgHeader * msg, size_t len);
  vector<char> cancelOrder(Ouch_MsgHeader * msg, size_t len);
  vector<char> modifyOrder(Ouch_MsgHeader * msg, size_t len);
  vector<char> replaceOrder(Ouch_MsgHeader * msg, size_t len);

  vector<char> constructOrderAccpeted(EnterOrder * eo, const order & o);
  vector<char> constructOrderRejected(EnterOrder * eo, const order & new_order);

  bool login(LoginRequest * req);

  uint64_t get_timestamp();

  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;
  chrono::system_clock::time_point start_of_day;
  char state;

  unordered_map<string, order> LiveOrders;
  unordered_map<string, order> DoneOrders;

};

#endif
