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
  LoginRequest * req = reinterpret_cast<LoginRequest*>(hdr);
  if (state != session_state::not_logged_in) constructLoginResponse(LoginResponseStatus::SessionInUse, req);
  else constructLoginResponse(LoginResponseStatus::Accepted, req);
}

void boe_session::constructLoginResponse(LoginResponseStatus status, LoginRequest * req){
  char * buf = (char*)calloc(300, sizeof(char));
  LoginResponse lr;
  lr.status = static_cast<uint8_t>(status);
  memset(lr.text, 0, sizeof(lr.text));
  lr.last_received_seq_num = 0;
  lr.no_unspecified_unit_replay = req->replay.no_unspecified_unit_replay;
  lr.number_of_units = 1;
  *((LoginResponse*)buf) = lr;
  buf->unit[0] = UnitSequence();
  buf->unit[0].number = 1;
  buf->unit[0].seq_num = 0;
  memcpy(&(buf->unit[1]), &(req->order_ack), (sizeof(ReturnBitfieldParamGroup)*4+20));
  auto packet = vector<char>(reinterpret_cast<const char*>(&lr), reinterpret_cast<const char*>(&lr)
                  + sizeof(lr) + sizeof(UnitSequence) + sizeof(ReturnBitfieldParamGroup) * 4 + 20);
  pending_out_messages.push_back(packet);
}

void boe_session::market_logic(){
}
