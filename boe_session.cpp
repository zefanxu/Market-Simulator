#include "session.h"

boe_session::boe_session(){

}

boe_session::boe_session(double random_reject_rate){
  this->random_reject_rate = random_reject_rate;
}

void boe_session::handle_packet(char * packet, size_t len){

}

void boe_session::market_logic(){

}

boe_session::~boe_session(){

}
