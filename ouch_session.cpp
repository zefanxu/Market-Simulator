#include "ouch_session.h"

string ouch_session::parse_packet(char * packet, size_t len){
  MsgHeader * msg_h = reinterpret_cast<MsgHeader*>(packet);
  switch (msg_h->packet_type){
    case(static_cast<char>(PacketType::LoginRequest)):
      return handle_login_request(msg_h, len);
    case(static_cast<char>(PacketType::LogoutRequest)):
      return handle_logout_request(msg_h, len);
    case(static_cast<char>(PacketType::ClientHeartbeat)):
      return handle_client_heartbeat(msg_h, len);
    default:
      return string();
  }
}

string ouch_session::handle_login_request(MsgHeader * packet, size_t len){
  LoginRequest * r = reinterpret_cast<LoginRequest*>(packet);
  if (login(r)){
    LoginAccepted la;
    //TODO: session and seq num assignment logic
    strncpy(la.session, "         0", sizeof(la.session));
    strncpy(la.seq_num, "                   0", sizeof(la.seq_num));
    return string(reinterpret_cast<const char*>(&la), sizeof(la));
  }else{
    LoginRejected lj;
    lj.reason = 'A';
    return string(reinterpret_cast<const char*>(&lj), sizeof(lj));
  }
}

string ouch_session::handle_logout_request(MsgHeader * packet, size_t len){
  state = ouch_state::not_logged_in;
  return string();
}

string ouch_session::handle_client_heartbeat(MsgHeader * packet, size_t len){
  last_recv_heartbeat = clock();
  if (state != ouch_state::not_logged_in)
    state = ouch_state::losing_heartbeat;
  return string();
}

bool ouch_session::login(LoginRequest * req){
  //TODO: login logic
  if (state == ouch_state::not_logged_in){
    state = ouch_state::logged_in;
    return true;
  }
  else{
    return false;
  }
}

ouch_session::ouch_session(){
  init();
}

void ouch_session::init(){
  state = ouch_state::not_logged_in;
  time_t curr_time = time(NULL);
  last_send_heartbeat = curr_time;
  last_recv_heartbeat = curr_time;
}

string ouch_session::heartbeat(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == ouch_state::not_logged_in) return string();
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    return string(reinterpret_cast<const char*>(&h), sizeof(h));
  }
  return string();
}
