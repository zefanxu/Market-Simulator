#include "boe_messages.h"
#include "order.h"

using namespace evt::boe;

boe_order::boe_order(NewOrder* no){
  parse_order(no);
}

void boe_order::parse_order(NewOrder* no){
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

bool boe_order::still_live(){
  return true;
}
