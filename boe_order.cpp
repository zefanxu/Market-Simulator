#include "boe_messages.h"
#include "order.h"

using namespace evt::boe;

boe_order::boe_order(NewOrder* no){
  parse_order(no);
}

void boe_order::parse_order(NewOrder* no){
  token = no->token;
  recv_order_time;
  time_in_force = no->time_in_force;
  remain_time_in_force = INT_MAX;
  remaining_qty = no->qty;
  executed_qty = 0;
  strncpy(symbol, no->symbol, sizeof(symbol));
  side = no->side;
  strncpy(firm, no->firm, sizeof(firm));
  capacity = no->capacity;
}

bool boe_order::still_live(){
  return true;
}
