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
  void setLogger(evtsim::Logger * l);

protected:
  virtual void heartbeat_logic()=0;
  virtual void cancel_logic()=0;
  virtual void execution_logic()=0;
  virtual void modify_logic()=0;
  virtual void replace_logic()=0;

  time_t last_send_heartbeat;
  time_t last_recv_heartbeat;

  char state;
  evtsim::Logger * l;
};

class boe_session : public session{
public:
  boe_session();
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

  virtual void heartbeat_logic();
  virtual void execution_logic();
  virtual void cancel_logic();
  virtual void modify_logic();
  virtual void replace_logic(){};

  void constructLoginResponse(boe::LoginResponseStatus status, boe::LoginRequest * req);
  void constructOrderAccpeted(Boe_Order & new_order);
  void constructOrderRejected(boe::NewOrder * no);
  void constructOrderExecuted(Boe_Order & curr_order);
  void constructCancelRejected(boe::Token t, boe::Reason r);
  void constructOrderCanceled(boe::Token t);
  void constructModifyRejected(boe::Token t, boe::Reason r);
  void constructOrderModified(const Boe_Order & bo);

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
  ouch_session();
  virtual ~ouch_session();
  virtual void handle_packet(char * packet, size_t len);

private:
  void init();
  bool validate(ouch::MsgHeader* msg_h, size_t len);
  bool validate_login_request(ouch::MsgHeader* msg_h, size_t len);
  bool validate_logout_request(ouch::MsgHeader* msg_h, size_t len);
  bool validate_client_heartbeat(ouch::MsgHeader* msg_h, size_t len);
  bool validate_enterOrder(ouch::MsgHeader * packet, size_t len);
  bool validate_cancelOrder(ouch::MsgHeader* packet, size_t len);
  bool validate_modifyOrder(ouch::MsgHeader* packet, size_t len);
  bool validate_replaceOrder(ouch::MsgHeader* packet, size_t len);

  void handle_login_request(ouch::MsgHeader * packet, size_t len);
  void handle_logout_request(ouch::MsgHeader * packet, size_t len);
  void handle_client_heartbeat(ouch::MsgHeader * packet, size_t len);
  void handle_message(ouch::MsgHeader* packet, size_t len);

  void enterOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void cancelOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void modifyOrder(ouch::Ouch_MsgHeader * msg, size_t len);
  void replaceOrder(ouch::Ouch_MsgHeader * msg, size_t len);

  virtual void heartbeat_logic();
  virtual void cancel_logic();
  virtual void execution_logic();
  virtual void modify_logic();
  virtual void replace_logic();

  void constructOrderAccpeted(const Ouch_Order & o);
  void constructOrderRejected(char reason, ouch::Token t);
  void constructOrderCanceled(uint32_t dec_qty, char reason, ouch::Token t);
  void constructOrderModified(uint32_t remaining_qty, const Ouch_ModifyOrderReq & mo);
  void constructOrderExecuted(Ouch_Order & o);
  void constructOrderReplaced(const Ouch_ReplaceOrderReq & ro, const Ouch_Order & new_order);

  uint64_t get_timestamp();

  unordered_map<string, Ouch_Order> active_orders;
  unordered_map<string, Ouch_Order> finished_orders;
  vector<Ouch_CancelOrderReq> pending_cancel;
  vector<Ouch_ModifyOrderReq> pending_modify;
  vector<Ouch_ReplaceOrderReq> pending_replace;

  chrono::system_clock::time_point start_of_day;
};

#endif
