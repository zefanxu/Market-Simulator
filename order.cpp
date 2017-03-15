#include "evtsim_messages.h"
#include "order.h"

using namespace evt::ouch;

order::order(){
  recv_order_time = time(NULL);
  orderID = get_timestamp();
}

order::order(EnterOrder & eo){
  recv_order_time = time(NULL);
  orderID = get_timestamp();
  parse_order(eo);
}

void order::parse_order(EnterOrder & eo){
  time_in_force = eo.time_in_force;
  remain_time_in_force = time_in_force;
  token = eo.token;
  qty = eo.qty;
  price = eo.price;
  min_qty = eo.min_qty;
  strncpy(symbol, eo.symbol, sizeof(symbol));
  state = OrderState::Live;
  side = eo.side;
}

bool order::still_live(){
  if ((time_in_force != IOC_time) and (time_in_force != market_hours) and (time_in_force != system_hours)){
    auto curr_time = time(NULL);
    remain_time_in_force = time_in_force - (curr_time - recv_order_time);
  }
  if ((qty <= 0) or (remain_time_in_force <= 0) or (state == OrderState::Dead)){
    state = OrderState::Dead;
    return false;
  }
  return true;
}
