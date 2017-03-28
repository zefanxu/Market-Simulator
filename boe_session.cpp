#include "session.h"
using namespace evt::boe;

boe_session::boe_session(){
  init();
}

boe_session::~boe_session(){
}

boe_session::boe_session(double random_reject_rate){
  init();
  this->random_reject_rate = random_reject_rate;
}

void boe_session::init(){
  state = session_state::not_logged_in;
  time_t curr_time = time(NULL);
  last_send_heartbeat = curr_time;
  last_recv_heartbeat = curr_time;
}

void boe_session::handle_packet(char * packet, size_t len){
  MsgHeader * hdr = reinterpret_cast<MsgHeader*>(packet);
  switch (hdr->type) {
    case static_cast<char>(MsgType::LoginRequest):
      handle_login_request(hdr, len);
      break;
    case static_cast<char>(MsgType::ClientHeartbeat):
      handle_client_heartbeat(hdr,len);
      break;
    case static_cast<char>(MsgType::NewOrder):
      enterOrder(hdr, len);
      break;
  }
}

void boe_session::enterOrder(MsgHeader * hdr, size_t len){

}

void boe_session::handle_client_heartbeat(MsgHeader* hdr, size_t len){
  last_recv_heartbeat = clock();
}

void boe_session::handle_login_request(MsgHeader* hdr, size_t len){
  LoginRequest * req = reinterpret_cast<LoginRequest*>(hdr);
  if (state != session_state::not_logged_in) constructLoginResponse(LoginResponseStatus::SessionInUse, req);
  else constructLoginResponse(LoginResponseStatus::Accepted, req);
  state = session_state::logged_in;
}

void boe_session::constructLoginResponse(LoginResponseStatus status, LoginRequest * req){
  char * buf = (char*)calloc(300, sizeof(char));
  LoginResponse lr;
  lr.status = static_cast<uint8_t>(status);
  memset(lr.text, 0, sizeof(lr.text));
  lr.last_received_seq_num = 0;
  lr.no_unspecified_unit_replay = req->replay.no_unspecified_unit_replay;
  lr.number_of_units = 1;
  auto packet_begin = (LoginResponse*)buf;
  *(packet_begin) = lr;
  packet_begin->unit[0] = UnitSequence();
  packet_begin->unit[0].number = 1;
  packet_begin->unit[0].seq_num = 0;
  //echo back the param groups
  memcpy(&(packet_begin->unit[1]), &(req->order_ack), (sizeof(ReturnBitfieldParamGroup)*4+20));
  size_t packet_size = sizeof(lr) + sizeof(UnitSequence) + sizeof(ReturnBitfieldParamGroup) * 4 + 20;
  packet_begin->length = packet_size-2;
  auto packet = vector<char>(buf, buf + packet_size);
  pending_out_messages.push_back(packet);
}

uint64_t boe_session::get_timestamp(){
  chrono::milliseconds ms = chrono::duration_cast< milliseconds >(chrono::system_clock::now().time_since_epoch());
  return ms;
}

void boe_session::heartbeat_logic(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == session_state::not_logged_in) return;
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    auto packet = vector<char>(reinterpret_cast<const char*>(&h), reinterpret_cast<const char*>(&h) + sizeof(h));
    pending_out_messages.push_back(packet);
  }
}

void boe_session::market_logic(){
  heartbeat_logic();
}
