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
#include <sstream>
#include "evtsim_util.h"
#include "ouch_messages.h"
#include "boe_messages.h"
#include "order.h"
#include "behavior_manager.h"

namespace SessionState{
  constexpr char NotLoggedIn = 'N';
  constexpr char LoggedIn = 'L';
}

class session{
public:
  virtual ~session(){};
  virtual void market_logic();
  virtual void handle_packet(char* packet, size_t len)=0;
  void disconnect(){state = SessionState::NotLoggedIn;}
  void set_logger(evtsim::Logger * l);

  std::vector<std::vector<char>> pending_out_messages;
protected:
  virtual void heartbeat_logic()=0;
  virtual void cancel_logic()=0;
  virtual void execution_logic()=0;
  virtual void modify_logic()=0;
  virtual void replace_logic()=0;
  virtual string curr_status()=0;

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

  void handle_login_request(evt::boe::MsgHeader* hdr, size_t len);
  void handle_client_heartbeat(evt::boe::MsgHeader* hdr, size_t len);

  void enter_order(evt::boe::MsgHeader * msg, size_t len);
  void cancel_order(evt::boe::MsgHeader * msg, size_t len);
  void modify_order(evt::boe::MsgHeader * msg, size_t len);

  virtual void heartbeat_logic();
  virtual void execution_logic();
  virtual void cancel_logic();
  virtual void modify_logic();
  virtual void replace_logic(){};

  void construct_login_response(evt::boe::LoginResponseStatus status, evt::boe::LoginRequest * req);
  void construct_order_accepted(evtsim::Boe_Order & new_order);
  void construct_order_rejected(evt::boe::NewOrder * no);
  void construct_order_executed(evtsim::Boe_Order & curr_order, int64_t exe_qty=-1);
  void construct_cancel_rejected(evt::boe::Token t, evt::boe::Reason r);
  void construct_order_canceled(evt::boe::Token t);
  void construct_modify_rejected(evt::boe::Token t, evt::boe::Reason r);
  void construct_order_modified(const evtsim::Boe_Order & bo);

  uint64_t get_timestamp();
  virtual std::string curr_status();

  std::unordered_map<std::string, evtsim::Boe_Order> active_orders;
  std::unordered_map<std::string, evtsim::Boe_Order> finished_orders;
  std::vector<evtsim::Boe_ModifyOrderReq> pending_modify;
  std::vector<evtsim::Boe_CancelOrderReq> pending_cancel;
  std::vector<evtsim::Boe_Order> pending_replace;

  unsigned int seq_num;
};

class ouch_session : public session{
public:
  ouch_session(BehaviorManager * bm);
  virtual ~ouch_session();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();
  bool validate(evt::ouch::MsgHeader* msg_h, size_t len);
  bool validate_login_request(evt::ouch::MsgHeader* msg_h, size_t len);
  bool validate_logout_request(evt::ouch::MsgHeader* msg_h, size_t len);
  bool validate_client_heartbeat(evt::ouch::MsgHeader* msg_h, size_t len);
  bool validate_enter_order(evt::ouch::MsgHeader * packet, size_t len);
  bool validate_cancel_order(evt::ouch::MsgHeader* packet, size_t len);
  bool validate_modify_order(evt::ouch::MsgHeader* packet, size_t len);
  bool validate_replace_order(evt::ouch::MsgHeader* packet, size_t len);

  void handle_login_request(evt::ouch::MsgHeader * packet, size_t len);
  void handle_logout_request(evt::ouch::MsgHeader * packet, size_t len);
  void handle_client_heartbeat(evt::ouch::MsgHeader * packet, size_t len);
  void handle_message(evt::ouch::MsgHeader* packet, size_t len);

  void enter_order(evt::ouch::Ouch_MsgHeader * msg, size_t len);
  void cancel_order(evt::ouch::Ouch_MsgHeader * msg, size_t len);
  void modify_order(evt::ouch::Ouch_MsgHeader * msg, size_t len);
  void replace_order(evt::ouch::Ouch_MsgHeader * msg, size_t len);

  virtual void heartbeat_logic();
  virtual void cancel_logic();
  virtual void execution_logic();
  virtual void modify_logic();
  virtual void replace_logic();

  void construct_login_accepted(unsigned int session_num=0, unsigned int seq_num=0);
  void construct_login_rejected(evt::ouch::LoginRejectReason r);
  void construct_cancel_rejected(evt::ouch::Token t);
  void construct_order_accepted(const evtsim::Ouch_Order & o);
  void construct_order_rejected(evt::ouch::RejectedReason reason, evt::ouch::Token t);
  void construct_order_canceled(uint32_t dec_qty, evt::ouch::CancelReason reason, evt::ouch::Token t);
  void construct_order_modified(uint32_t remaining_qty, const evtsim::Ouch_ModifyOrderReq & mo);
  void construct_order_executed(evtsim::Ouch_Order & o, int64_t exe_qty=-1);
  void construct_order_replaced(const evtsim::Ouch_ReplaceOrderReq & ro, const evtsim::Ouch_Order & new_order);

  uint64_t get_timestamp();
  virtual std::string curr_status();

  std::unordered_map<std::string, evtsim::Ouch_Order> active_orders;
  std::unordered_map<std::string, evtsim::Ouch_Order> finished_orders;
  std::vector<evtsim::Ouch_CancelOrderReq> pending_cancel;
  std::vector<evtsim::Ouch_ModifyOrderReq> pending_modify;
  std::vector<evtsim::Ouch_ReplaceOrderReq> pending_replace;

  std::chrono::system_clock::time_point start_of_day;
};

#endif
