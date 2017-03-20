#include "evtsim_messages.h"
#include "order.h"

using namespace evt::ouch;

CancelOrderReq::CancelOrderReq(){
}

CancelOrderReq::CancelOrderReq(CancelOrder* co){
  parse_cancel_order(co);
}

void CancelOrderReq::parse_cancel_order(CancelOrder * co){
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
  cross_type = eo->cross_type;
  intermarket_sweep_eligibility = eo->intermarket_sweep_eligibility;
  strncpy(firm, eo->firm, sizeof(firm));
  display = eo->display;
  capacity = eo->capacity;
}

bool order::still_live(){
  if ((remaining_qty <= 0) or expired() or (state == OrderState::Dead)){
    state = OrderState::Dead;
    return false;
  }
  return true;
}

bool order::expired(){
  if ((time_in_force != IOC_time) and (time_in_force != market_hours) and (time_in_force != system_hours)){
    auto curr_time = time(NULL);
    remain_time_in_force = time_in_force - (curr_time - recv_order_time);
  }
  return (remain_time_in_force <= 0);
}

ModifyOrderReq::ModifyOrderReq(){
}

ModifyOrderReq::ModifyOrderReq(ModifyOrder * mo){
  parse_modify_order(mo);
}

void ModifyOrderReq::parse_modify_order(ModifyOrder * mo){
  token = mo -> token;
  req_qty = mo -> qty;
  new_side = mo -> side;
}

ReplaceOrderReq::ReplaceOrderReq(){
}

ReplaceOrderReq::ReplaceOrderReq(ReplaceOrder * ro){
  parse_replace_order(ro);
}

void ReplaceOrderReq::parse_replace_order(ReplaceOrder * ro){
  existing_token = ro -> existing_token;
  new_token = ro -> token;
  qty = ro -> qty;
  price = ro -> price;
  time_in_force = ro -> time_in_force;
  display = ro -> display;
  intermarket_sweep_eligibility = ro -> intermarket_sweep_eligibility;
  min_qty = ro -> min_qty;
}
