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

#define MAX_SHARES 999999

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
  void market_logic();
  void handle_packet(char * packet, size_t len);

  vector<vector<char>> pending_out_messages;

private:
  void handle_login_request(MsgHeader * packet, size_t len);
  void handle_logout_request(MsgHeader * packet, size_t len);
  void handle_client_heartbeat(MsgHeader * packet, size_t len);
  void handle_message(MsgHeader* packet, size_t len);

  void enterOrder(Ouch_MsgHeader * msg, size_t len);
  void cancelOrder(Ouch_MsgHeader * msg, size_t len);
  void modifyOrder(Ouch_MsgHeader * msg, size_t len);
  void replaceOrder(Ouch_MsgHeader * msg, size_t len);

  void heartbeat_logic();
  void cancel_logic();
  void execution_logic();
  void modify_logic();
  void replace_logic();

  void constructOrderAccpeted(const order & o);
  void constructOrderRejected(char reason, Token t);
  void constructOrderCanceled(uint32_t dec_qty, char reason, Token t);
  void constructOrderModified(uint32_t remaining_qty, const ModifyOrderReq & mo);
  void constructOrderExecuted(order & o);
  void constructOrderReplaced(const ReplaceOrderReq & ro, const order & new_order);

  uint64_t get_timestamp();

  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;
  chrono::system_clock::time_point start_of_day;
  char state;

  unordered_map<string, order> active_orders;
  unordered_map<string, order> finished_orders;
  vector<CancelOrderReq> pending_cancel;
  vector<ModifyOrderReq> pending_modify;
  vector<ReplaceOrderReq> pending_replace;
};

#endif
