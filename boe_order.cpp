#include "boe_messages.h"
#include "order.h"

using namespace evt::boe;
using namespace evtsim;
using namespace std;

Boe_Order::Boe_Order(NewOrder* no){
  parse_order(no);
}

Boe_Order::Boe_Order(){
}

void Boe_Order::parse_order(NewOrder* no){
  token = no->token;
  time_in_force = no->time_in_force;
  remain_time_in_force = 0xFFFFFF;
  remaining_qty = no->qty;
  executed_qty = 0;
  strncpy(symbol, no->symbol, sizeof(symbol));
  side = no->side;
  strncpy(firm, no->clearing_firm, sizeof(firm));
  capacity = no->capacity;
  min_qty = no->min_qty;
  price = no->price;
  display = no->display_indicator;
}

bool Boe_Order::still_active(){
  return (remaining_qty > 0 and remaining_qty <= 1000000);
}

Boe_CancelOrderReq::Boe_CancelOrderReq(){
}

Boe_CancelOrderReq::Boe_CancelOrderReq(CancelOrder * co){
  token = co->orig_token;
}

Boe_ModifyOrderReq::Boe_ModifyOrderReq(){
}

Boe_ModifyOrderReq::Boe_ModifyOrderReq(ModifyOrder * mo){
  parse_modify_order(mo);
}

void Boe_ModifyOrderReq::parse_modify_order(ModifyOrder * mo){
  token = mo->token;
  orig_token = mo->orig_token;
  qty = mo->qty;
  price = mo->price;
  order_type = mo->order_type;
  cancel_orig_on_reject = mo->cancel_orig_on_reject;
  exec_inst = mo->exec_inst;
  side = mo->side;
  strncpy(clearing_firm, mo->clearing_firm, 4);
}
