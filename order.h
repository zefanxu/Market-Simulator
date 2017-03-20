#pragma once
#include <boost/enum.hpp>
#include <time.h>
#include <stdlib.h>
#include "evtsim_messages.h"

constexpr uint32_t IOC_time = 0;
constexpr uint32_t market_hours = 99998;
constexpr uint32_t system_hours = 99999;

namespace evt{
namespace ouch{


class order{
public:
  order();
  order(EnterOrder* eo);
  void parse_order(EnterOrder* eo);
  bool still_live();

  time_t recv_order_time;
  uint32_t time_in_force;
  int32_t remain_time_in_force;
  uint32_t remaining_qty;
  int32_t price;
  uint32_t min_qty;
  char symbol[8];
  Token token;
  OrderState state;
  char side;
  char cross_type;
  char intermarket_sweep_eligibility;
  char firm[4];
  char display;
  char capacity;

  uint64_t orderID;
  uint64_t executed_qty;
};

class cancel_order{
public:
  cancel_order();
  cancel_order(CancelOrder* co);
  void parse_cancel_order(CancelOrder* co);

  Token token;
  uint32_t qty;
};

class modify_order{
public:
  modify_order();
  modify_order(ModifyOrder* mo);
  void parse_modify_order(ModifyOrder* mo);

  Token token;
  uint32_t req_qty;
  char new_side;
};

class replace_order{
public:
  replace_order();
  replace_order(ReplaceOrder* ro);
  void parse_replace_order(ReplaceOrder* ro);

  Token existing_token;
  Token new_token;
  uint32_t qty;
  uint32_t price;
  uint32_t time_in_force;
  char display;
  char intermarket_sweep_eligibility;
  uint32_t min_qty;
}

}
}
