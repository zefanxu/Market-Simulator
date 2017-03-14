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
    case(static_cast<char>(PacketType::UnsequencedData)):
      return parse_message(msg_h, len);
    default:
      return string();
  }
}

string ouch_session::parse_message(MsgHeader * packet, size_t len){
  Ouch_MsgHeader * ouch_msg_h = reinterpret_cast<Ouch_MsgHeader*>(packet);
  switch (ouch_msg_h->msg_type) {
    case(static_cast<char>(OutboundMsgType::EnterOrder)):
      return enterOrder(ouch_msg_h, len);
    case(static_cast<char>(OutboundMsgType::ReplaceOrder)):
      return replaceOrder(ouch_msg_h, len);
    case(static_cast<char>(OutboundMsgType::CancelOrder)):
      return cancelOrder(ouch_msg_h, len);
    case(static_cast<char>(OutboundMsgType::ModifyOrder)):
      return modifyOrder(ouch_msg_h, len);
    default:
      return string();
  }
}

string ouch_session::enterOrder(Ouch_MsgHeader * msg, size_t len){
  EnterOrder * eo = reinterpret_cast<EnterOrder*>(msg);
  OrderAccepted oa;
  oa.timestamp = get_timestamp();
  oa.clordid = eo.clordid;
  oa.side = eo.side;
  strncpy(oa.symbol, eo.symbol, sizeof(oa.symbol));
  oa.price = eo.price;
  oa.time_in_force = eo.time_in_force;
  strncpy(oa.firm, eo.firm, sizeof(oa.firm));
  oa.display = eo.display;
  oa.capacity = eo.capacity;
  oa.intermarket_sweep_eligibility = eo.intermarket_sweep_eligibility;
  oa.min_qty = eo.min_qty;
  oa.cross_type = eo.cross_type;
  //TODO: hardcoded BBO weight indicator
  oa.bbo_weight_indicator = '2';
  oa.order_state = static_cast<char>(OrderState::Live);
  //TODO: order_reference_number
  oa.order_reference_number = 1;
  oa.format();
  return string(reinterpret_cast<const char*>(&oa), sizeof(oa));
}

uint64_t get_timestamp(){
  auto curr_time = chrono::high_resolution_clock::now();
  chrono::duration<uint64_t, std::nano> diff = curr_time - start_of_day;
  return diff.count();
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

  //get the start of the day time
  time_t t1 = time(NULL);
  struct tm tms;
  tms.tm_hour = 0;
  tms.tm_min = 0;
  tms.tm_sec = 0;
  t1 = mktime(&tms);
  start_of_day = chrono::system_clock::front_time_t(t1);
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
