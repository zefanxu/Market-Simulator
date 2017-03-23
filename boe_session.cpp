#include "session.h"
using namespace evt::boe;

boe_session::boe_session(){
}

boe_session::~boe_session(){
}


boe_session::boe_session(double random_reject_rate){
  this->random_reject_rate = random_reject_rate;
}

void boe_session::handle_packet(char * packet, size_t len){
  MsgHeader * hdr = reinterpret_cast<MsgHeader*>(packet);
  switch (hdr->type) {
    case static_cast<char>(MsgType::LoginRequest):
      handle_login_request(hdr, len);
  }

}

void boe_session::handle_login_request(MsgHeader* hdr, size_t len){

}


void boe_session::market_logic(){
}
