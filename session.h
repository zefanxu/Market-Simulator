#ifndef _OUCH_SESSION_H
#define _OUCH_SESSION_H
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <time.h>
#include <regex>
#include <chrono>
#include "evtsim_util.h"
#include "ouch_messages.h"
#include "boe_messages.h"
#include "order.h"
#include "behavior_manager.h"

using namespace evt;
namespace session_state{
  constexpr char not_logged_in = 'N';
  constexpr char logged_in = 'L';
}

class session{
public:
  virtual ~session(){};
  virtual void market_logic();
  virtual void handle_packet(char* packet, size_t len)=0;
  void disconnect(){state = session_state::not_logged_in;}
  vector<vector<char>> pending_out_messages;
  void set_logger(evtsim::Logger * l);

protected:
  virtual void heartbeat_logic()=0;
  virtual void cancel_logic()=0;
  virtual void execution_logic()=0;
  virtual void modify_logic()=0;
  virtual void replace_logic()=0;

  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;

  BehaviorManager * _behavior;
  char state;
  evtsim::Logger * l;
};

class boe_session : public session{
public:
  boe_session(BehaviorManager * bm);
  virtual ~boe_session();
  virtual void market_logic();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();

  void handle_login_request(boe::MsgHeader* hdr, size_t len);
  void handle_client_heartbeat(boe::MsgHeader* hdr, size_t len);

  void enter_order(boe::MsgHeader * msg, size_t len);
  void cancel_order(boe::MsgHeader * msg, size_t len);
  void modify_order(boe::MsgHeader * msg, size_t len);

  virtual void heartbeat_logic();
  virtual void execution_logic();
  virtual void cancel_logic();
  virtual void modify_logic();
  virtual void replace_logic(){};

  void construct_login_response(boe::LoginResponseStatus status, boe::LoginRequest * req);
  void construct_order_accepted(Boe_Order & new_order);
  void construct_order_rejected(boe::NewOrder * no);
  void construct_order_executed(Boe_Order & curr_order, int64_t exe_qty=0);
  void construct_cancel_rejected(boe::Token t, boe::Reason r);
  void construct_order_canceled(boe::Token t);
  void construct_modify_rejected(boe::Token t, boe::Reason r);
  void construct_order_modified(const Boe_Order & bo);

  uint64_t get_timestamp();

  unordered_map<string, Boe_Order> active_orders;
  unordered_map<string, Boe_Order> finished_orders;
  vector<Boe_ModifyOrderReq> pending_modify;
  vector<Boe_CancelOrderReq> pending_cancel;
  vector<Boe_Order> pending_replace;

  unsigned int seq_num;
};

class ouch_session : public session{
public:
  ouch_session(BehaviorManager * bm);
  virtual ~ouch_session();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();
  bool validate(ouch::MsgHeader* msg_h, size_t len);
  bool validate_login_request(ouch::MsgHeader* msg_h, size_t len);
  bool validate_logout_request(ouch::MsgHeader* msg_h, size_t len);
  bool validate_client_heartbeat(ouch::MsgHeader* msg_h, size_t len);
  bool validate_enter_order(ouch::MsgHeader * packet, size_t len);
  bool validate_cancel_order(ouch::MsgHeader* packet, size_t len);
  bool validate_modify_order(ouch::MsgHeader* packet, size_t len);
  bool validate_replace_order(ouch::MsgHeader* packet, size_t len);

  void handle_login_request(ouch::MsgHeader * packet, size_t len);
  void handle_logout_request(ouch::MsgHeader * packet, size_t len);
  void handle_client_heartbeat(ouch::MsgHeader * packet, size_t len);
  void handle_message(ouch::MsgHeader* packet, size_t len);

  void enter_order(ouch::Ouch_MsgHeader * msg, size_t len);
  void cancel_order(ouch::Ouch_MsgHeader * msg, size_t len);
  void modify_order(ouch::Ouch_MsgHeader * msg, size_t len);
  void replace_order(ouch::Ouch_MsgHeader * msg, size_t len);

  virtual void heartbeat_logic();
  virtual void cancel_logic();
  virtual void execution_logic();
  virtual void modify_logic();
  virtual void replace_logic();

  void construct_order_accepted(const Ouch_Order & o);
  void construct_order_rejected(char reason, ouch::Token t);
  void construct_order_canceled(uint32_t dec_qty, char reason, ouch::Token t);
  void construct_order_modified(uint32_t remaining_qty, const Ouch_ModifyOrderReq & mo);
  void construct_order_executed(Ouch_Order & o, int64_t exe_qty=0);
  void construct_order_replaced(const Ouch_ReplaceOrderReq & ro, const Ouch_Order & new_order);

  uint64_t get_timestamp();

  unordered_map<string, Ouch_Order> active_orders;
  unordered_map<string, Ouch_Order> finished_orders;
  vector<Ouch_CancelOrderReq> pending_cancel;
  vector<Ouch_ModifyOrderReq> pending_modify;
  vector<Ouch_ReplaceOrderReq> pending_replace;

  chrono::system_clock::time_point start_of_day;
};

#endif
