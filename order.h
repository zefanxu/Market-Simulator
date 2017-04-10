#pragma once
#include <boost/enum.hpp>
#include <time.h>
#include <stdlib.h>
#include "ouch_messages.h"
#include "boe_messages.h"

using namespace evt;

class order{
public:
  order();

  virtual bool still_live()=0;

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
  Boe_Order(boe::NewOrder* no);
  void parse_order(boe::NewOrder* no);

  virtual bool still_live();

  boe::Token token;
  uint64_t price;
};

class Ouch_Order : public order{
public:
  Ouch_Order();
  Ouch_Order(ouch::EnterOrder* eo);
  void parse_order(ouch::EnterOrder* eo);

  virtual bool still_live();
  bool expired();

  uint32_t time_in_force;
  char intermarket_sweep_eligibility;
  int32_t price;
  char cross_type;
  ouch::Token token;
  ouch::OrderState state;
};

class Ouch_CancelOrderReq{
public:
  Ouch_CancelOrderReq();
  Ouch_CancelOrderReq(ouch::CancelOrder* co);
  void parse_cancel_order(ouch::CancelOrder* co);

  ouch::Token token;
  uint32_t qty;
};

class Boe_CancelOrderReq{
public:
  Boe_CancelOrderReq();
  Boe_CancelOrderReq(boe::CancelOrder* co);

  boe::Token token;
};

class Ouch_ModifyOrderReq{
public:
  Ouch_ModifyOrderReq();
  Ouch_ModifyOrderReq(ouch::ModifyOrder* mo);
  void parse_modify_order(ouch::ModifyOrder* mo);

  ouch::Token token;
  uint32_t req_qty;
  char new_side;
};

class Boe_ModifyOrderReq{
public:
  Boe_ModifyOrderReq();
  Boe_ModifyOrderReq(boe::ModifyOrder* mo);
  void parse_modify_order(boe::ModifyOrder* mo);

  boe::Token token;
  boe::Token orig_token;
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
  Ouch_ReplaceOrderReq(ouch::ReplaceOrder* ro);
  void parse_replace_order(ouch::ReplaceOrder* ro);

  ouch::Token existing_token;
  ouch::Token new_token;
  uint32_t qty;
  uint32_t price;
  uint32_t time_in_force;
  char display;
  char intermarket_sweep_eligibility;
  uint32_t min_qty;
};
