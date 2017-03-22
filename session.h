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

class session{
public:
  virtual void market_logic()=0;
  virtual void handle_packet(char* packet, size_t len)=0;
  vector<vector<char>> pending_out_messages;

protected:
  char state;
};

class ouch_session : public session{
public:
  ouch_session();
  ouch_session(double random_reject_rate);
  virtual void market_logic();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();
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
  bool order_random_reject();

  unordered_map<string, order> active_orders;
  unordered_map<string, order> finished_orders;
  vector<CancelOrderReq> pending_cancel;
  vector<ModifyOrderReq> pending_modify;
  vector<ReplaceOrderReq> pending_replace;

  double random_reject_rate; //0 <= x <= 1, default to 0.3333
  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;
  chrono::system_clock::time_point start_of_day;
};

#endif
