#include "ouch_session.h"

string ouch_session::parse_packet(char * packet, size_t len){
  MsgHeader * msg_h = reinterpret_cast<MsgHeader*>(packet);
  cout << outbound_to_string(msg_h) << endl;
  switch (msg_h->packet_type){
    case('L'):
      return handle_login_request(msg_h, len);
    case('O'):
      return handle_logout_request(msg_h, len);
    default:
      return string();
  }
}

string ouch_session::handle_login_request(MsgHeader * packet, size_t len){
  LoginRequest * r = reinterpret_cast<LoginRequest*>(packet);
  if (login(r)){
    LoginAccepted la;
    strncpy(la.session, "         0", sizeof(la.session));
    strncpy(la.seq_num, "                   0", sizeof(la.seq_num));
    cout << inbound_to_string(reinterpret_cast<MsgHeader*>(&la)) << endl;
    return string(reinterpret_cast<const char*>(&la), sizeof(la));
  }else{
    LoginRejected lj;
    lj.reason = 'A';
    cout << inbound_to_string(reinterpret_cast<MsgHeader*>(&lj)) << endl;
    return string(reinterpret_cast<const char*>(&lj), sizeof(lj));
  }
}

string ouch_session::handle_logout_request(MsgHeader * packet, size_t len){
  LogoutRequest * r = reinterpret_cast<LogoutRequest*>(packet);
  state = ouch_state::not_logged_in;
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

void init(){

}
