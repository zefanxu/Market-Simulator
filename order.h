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
  bool expired();

  time_t recv_order_time;

  char time_in_force;
  uint32_t remaining_qty;
  uint64_t orderID;
  uint64_t executed_qty;
  char symbol[8];
  char side;
  char firm[4];
  char capacity;
};

class boe_order : public order{
public:
  boe_order(boe::NewOrder* no);
  void parse_order(boe::NewOrder* no);

  virtual bool still_live();

  boe::Token token;
  uint64_t price;
};

class ouch_order : public order{
public:
  ouch_order(ouch::EnterOrder* eo);
  void parse_order(ouch::EnterOrder* eo);

  virtual bool still_live();

  char display;
  uint32_t time_in_force;
  int32_t remain_time_in_force;
  char intermarket_sweep_eligibility;
  int32_t price;
  char cross_type;
  ouch::Token token;
  ouch::OrderState state;
  uint32_t min_qty;
};

class CancelOrderReq{
public:
  CancelOrderReq();
  CancelOrderReq(ouch::CancelOrder* co);
  void parse_cancel_order(ouch::CancelOrder* co);

  ouch::Token token;
  uint32_t qty;
};

class ModifyOrderReq{
public:
  ModifyOrderReq();
  ModifyOrderReq(ouch::ModifyOrder* mo);
  void parse_modify_order(ouch::ModifyOrder* mo);

  ouch::Token token;
  uint32_t req_qty;
  char new_side;
};

class ReplaceOrderReq{
public:
  ReplaceOrderReq();
  ReplaceOrderReq(ouch::ReplaceOrder* ro);
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
