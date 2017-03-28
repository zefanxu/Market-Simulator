#include "boe_messages.h"
#include "order.h"

using namespace evt::boe;

order::order(NewOrder* no){
  parse_order(no);
}

order::parse_order(NewOrder* no){

}
