#pragma once
#include <boost/enum.hpp>
#include <time.h>
#include <stdlib.h>
#include "ouch_messages.h"
#include "boe_messages.h"

namespace evtsim{
class order{
public:
  order();

  virtual bool still_active() =0;

  time_t recv_order_time;
  char time_in_force;
  int64_t remaining_qty;
  uint64_t orderID;
  int64_t executed_qty;
  char symbol[8];
  char side;
  char firm[4];
  char capacity;
  char display;
  int32_t remain_time_in_force;
  uint32_t min_qty;
};

class Boe_Order : public order{
public:
  Boe_Order();
  Boe_Order(evt::boe::NewOrder* no);
  void parse_order(evt::boe::NewOrder* no);

  virtual bool still_active() ;

  evt::boe::Token token;
  uint64_t price;
};

class Ouch_Order : public order{
public:
  Ouch_Order();
  Ouch_Order(evt::ouch::EnterOrder* eo);
  void parse_order(evt::ouch::EnterOrder* eo);

  virtual bool still_active();
  bool expired();

  uint32_t time_in_force;
  char intermarket_sweep_eligibility;
  int32_t price;
  char cross_type;
  evt::ouch::Token token;
  evt::ouch::OrderState state;
};

class Ouch_CancelOrderReq{
public:
  Ouch_CancelOrderReq();
  Ouch_CancelOrderReq(evt::ouch::CancelOrder* co);
  void parse_cancel_order(evt::ouch::CancelOrder* co);

  evt::ouch::Token token;
  uint32_t qty;
};

class Boe_CancelOrderReq{
public:
  Boe_CancelOrderReq();
  Boe_CancelOrderReq(evt::boe::CancelOrder* co);

  evt::boe::Token token;
};

class Ouch_ModifyOrderReq{
public:
  Ouch_ModifyOrderReq();
  Ouch_ModifyOrderReq(evt::ouch::ModifyOrder* mo);
  void parse_modify_order(evt::ouch::ModifyOrder* mo);

  evt::ouch::Token token;
  uint32_t req_qty;
  char new_side;
};

class Boe_ModifyOrderReq{
public:
  Boe_ModifyOrderReq();
  Boe_ModifyOrderReq(evt::boe::ModifyOrder* mo);
  void parse_modify_order(evt::boe::ModifyOrder* mo);

  evt::boe::Token token;
  evt::boe::Token orig_token;
  char clearing_firm[4];
  uint32_t qty;
  uint64_t price;
  char order_type;
  char cancel_orig_on_reject;
  char exec_inst;
  char side;
};

class Ouch_ReplaceOrderReq{
public:
  Ouch_ReplaceOrderReq();
  Ouch_ReplaceOrderReq(evt::ouch::ReplaceOrder* ro);
  void parse_replace_order(evt::ouch::ReplaceOrder* ro);

  evt::ouch::Token existing_token;
  evt::ouch::Token new_token;
  uint32_t qty;
  uint32_t price;
  uint32_t time_in_force;
  char display;
  char intermarket_sweep_eligibility;
  uint32_t min_qty;
};
}
