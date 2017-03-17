#include "evtsim_messages.h"
#include "order.h"

using namespace evt::ouch;

cancel_order::cancel_order(){
}

cancel_order::cancel_order(CancelOrder* co){
  parse_cancel_order(co);
}

void cancel_order::parse_cancel_order(CancelOrder * co){
  token = co->token;
  qty = co->qty;
}

order::order(){
  recv_order_time = time(NULL);
  orderID = rand() * rand();
}

order::order(EnterOrder* eo){
  recv_order_time = time(NULL);
  orderID = rand() * rand();
  parse_order(eo);
}

void order::parse_order(EnterOrder* eo){
  time_in_force = eo->time_in_force;
  remain_time_in_force = time_in_force;
  token = eo->token;
  remaining_qty = eo->qty;
  executed_qty = 0;
  price = eo->price;
  min_qty = eo->min_qty;
  strncpy(symbol, eo->symbol, sizeof(symbol));
  state = OrderState::Live;
  side = eo->side;
}

bool order::still_live(){
  if ((time_in_force != IOC_time) and (time_in_force != market_hours) and (time_in_force != system_hours)){
    auto curr_time = time(NULL);
    remain_time_in_force = time_in_force - (curr_time - recv_order_time);
  }
  if ((remaining_qty <= 0) or (remain_time_in_force <= 0) or (state == OrderState::Dead)){
    state = OrderState::Dead;
    return false;
  }
  return true;
}

modify_order::modify_order(){
}

modify_order::modify_order(ModifyOrder * mo){
  parse_modify_order(mo);
}

void modify_order::parse_modify_order(ModifyOrder * mo){
  token = mo -> token;
  req_qty = mo -> qty;
  new_side = mo -> side;
}
