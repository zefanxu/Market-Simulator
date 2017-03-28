#include "order.h"

order::order(){
  recv_order_time = time(NULL);
  orderID = rand() * rand();
}


