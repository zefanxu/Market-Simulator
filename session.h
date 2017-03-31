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
#include "ouch_messages.h"
#include "boe_messages.h"
#include "order.h"

using namespace evt;
namespace session_state{
  constexpr char not_logged_in = 'N';
  constexpr char logged_in = 'L';
  constexpr char losing_heartbeat = '1';
}

class session{
public:
  virtual ~session(){};
  virtual void market_logic()=0;
  virtual void handle_packet(char* packet, size_t len)=0;
  vector<vector<char>> pending_out_messages;

protected:
  bool order_random_reject();
  char state;
  double random_reject_rate; //0 <= x <= 1, default to 0.3333
};

class boe_session : public session{
public:
  boe_session();
  boe_session(double random_reject_rate);
  virtual ~boe_session();
  virtual void market_logic();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();

  void handle_login_request(boe::MsgHeader* hdr, size_t len);
  void handle_client_heartbeat(boe::MsgHeader* hdr, size_t len);

  void enterOrder(boe::MsgHeader * msg, size_t len);
  void cancelOrder(boe::MsgHeader * msg, size_t len);
  void modifyOrder(boe::MsgHeader * msg, size_t len);

  void heartbeat_logic();
  void execution_logic();
  void cancel_logic();
  void modify_logic();

  void constructLoginResponse(boe::LoginResponseStatus status, boe::LoginRequest * req);
  void constructOrderAccpeted(boe_order & new_order);
  void constructOrderRejected(boe::NewOrder * no);
  void constructOrderExecuted(boe_order & curr_order);
  void constructCancelRejected(boe::Token t, boe::Reason r);
  void constructOrderCanceled(boe::Token t);

  uint64_t get_timestamp();

  unordered_map<string, boe_order> active_orders;
  unordered_map<string, boe_order> finished_orders;
  vector<boe_order> pending_modify;
  vector<Boe_CancelOrderReq> pending_cancel;
  vector<boe_order> pending_replace;

  unsigned int seq_num;
  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;

};

class ouch_session : public session{
public:
  ouch_session();
  ouch_session(double random_reject_rate);
  virtual ~ouch_session();
  virtual void market_logic();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();
  void handle_login_request(ouch::MsgHeader * packet, size_t len);
  void handle_logout_request(ouch::MsgHeader * packet, size_t len);
  void handle_client_heartbeat(ouch::MsgHeader * packet, size_t len);
  void handle_message(ouch::MsgHeader* packet, size_t len);

  void enterOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void cancelOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void modifyOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void replaceOrder(ouch::Ouch_MsgHeader * msg, size_t len);

  void heartbeat_logic();
  void cancel_logic();
  void execution_logic();
  void modify_logic();
  void replace_logic();

  void constructOrderAccpeted(const ouch_order & o);
  void constructOrderRejected(char reason, ouch::Token t);
  void constructOrderCanceled(uint32_t dec_qty, char reason, ouch::Token t);
  void constructOrderModified(uint32_t remaining_qty, const Ouch_ModifyOrderReq & mo);
  void constructOrderExecuted(ouch_order & o);
  void constructOrderReplaced(const Ouch_ReplaceOrderReq & ro, const ouch_order & new_order);

  uint64_t get_timestamp();


  unordered_map<string, ouch_order> active_orders;
  unordered_map<string, ouch_order> finished_orders;
  vector<Ouch_CancelOrderReq> pending_cancel;
  vector<Ouch_ModifyOrderReq> pending_modify;
  vector<Ouch_ReplaceOrderReq> pending_replace;

  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;
  chrono::system_clock::time_point start_of_day;
};

#endif
