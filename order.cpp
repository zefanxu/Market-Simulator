#include "order.h"

order::order(){
  recv_order_time = time(NULL);
  orderID = rand() * rand();
}

bool order::expired(){
  if ((time_in_force != TimeInForce::Ioc) and (time_in_force != TimeInForce::Market) and (time_in_force != TimeInForce::System)){
    auto curr_time = time(NULL);
    remain_time_in_force = time_in_force - (curr_time - recv_order_time);
  }
  return (remain_time_in_force <= 0);
}
