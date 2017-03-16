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
  order(EnterOrder & eo);
  void parse_order(EnterOrder & eo);
  bool still_live();

  time_t recv_order_time;
  uint32_t time_in_force;
  int32_t remain_time_in_force;
  uint32_t qty;
  int32_t price;
  uint32_t min_qty;
  char symbol[8];
  Token token;
  OrderState state;
  char side;

  uint64_t orderID;
};

class cancel_order{
public:
  cancel_order();
  cancel_order(CancelOrder* co);
  void parse_cancel_order(CancelOrder* co);

  Token token;
  uint32_t qty;
}

  }
}
