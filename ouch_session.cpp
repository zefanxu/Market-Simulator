#include "ouch_session.h"

vector<char> ouch_session::parse_packet(char * packet, size_t len){
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
      return vector<char>();
  }
}

vector<char> ouch_session::parse_message(MsgHeader * packet, size_t len){
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
      return vector<char>();
  }
}

vector<char> ouch_session::replaceOrder(Ouch_MsgHeader * msg, size_t len){
  return vector<char>();
}

vector<char> ouch_session::cancelOrder(Ouch_MsgHeader * msg, size_t len){
  return vector<char>();
}

vector<char> ouch_session::modifyOrder(Ouch_MsgHeader * msg, size_t len){
  return vector<char>();
}

vector<char> ouch_session::execute_logic(){
  auto ret = heartbeat();
  if (ret.size()) return ret;
  for (auto & it : LiveOrders){
    order & each_order = it.second;
    if (each_order.still_live() and (each_order.qty > each_order.min_qty)){
      uint32_t exe_qty = each_order.qty;
      Executed ex;
      ex.timestamp = get_timestamp();
      ex.token = each_order.token;
      ex.executed_qty = exe_qty;
      ex.execution_price = each_order.price;
      ex.liquidity_flag = 'R';
      ex.match_number = each_order.orderID;
      each_order.qty -= exe_qty;
      ex.to_network();
      return vector<char>(reinterpret_cast<const char*>(&ex), reinterpret_cast<const char*>(&ex) + sizeof(ex));
    }
    else{
      DoneOrders[string(it.first)] = it.second;
      LiveOrders.erase(it.first);
    }
  }
  return std::vector<char>();
}

vector<char> ouch_session::enterOrder(Ouch_MsgHeader * msg, size_t len){
  EnterOrder * eo = reinterpret_cast<EnterOrder*>(msg);
  eo->from_network();

  //TODO: randomly reject orders
  if (!(rand() % 5) or (state != ouch_state::logged_in)){
    return constructOrderRejected(eo);
  }
  if ((LiveOrders.find(string(eo->token.val)) != LiveOrders.end()) or
      (DoneOrders.find(string(eo->token.val)) != DoneOrders.end())){
    order new_order = order(*eo);
    LiveOrders[string(eo->token.val)] = new_order;
    return constructOrderAccpeted(eo, new_order);
  }
  //ignore enter order
  return vector<char>();
}

uint64_t ouch_session::get_timestamp(){
  auto curr_time = chrono::high_resolution_clock::now();
  chrono::duration<uint64_t, std::nano> diff = curr_time - start_of_day;
  return diff.count();
}

vector<char> ouch_session::handle_login_request(MsgHeader * packet, size_t len){
  LoginRequest * r = reinterpret_cast<LoginRequest*>(packet);
  if (login(r)){
    LoginAccepted la;
    //TODO: session and seq num assignment logic
    strncpy(la.session, "         0", sizeof(la.session));
    strncpy(la.seq_num, "                   0", sizeof(la.seq_num));
    return vector<char>(reinterpret_cast<const char*>(&la), reinterpret_cast<const char*>(&la) + sizeof(la));
  }else{
    LoginRejected lj;
    lj.reason = 'A';
    return vector<char>(reinterpret_cast<const char*>(&lj), reinterpret_cast<const char*>(&lj) + sizeof(lj));
  }
}

vector<char> ouch_session::handle_logout_request(MsgHeader * packet, size_t len){
  state = ouch_state::not_logged_in;
  return vector<char>();
}

vector<char> ouch_session::handle_client_heartbeat(MsgHeader * packet, size_t len){
  last_recv_heartbeat = clock();
  if (state != ouch_state::not_logged_in)
    state = ouch_state::losing_heartbeat;
  return vector<char>();
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
  start_of_day = chrono::system_clock::from_time_t(t1);
}

vector<char> ouch_session::heartbeat(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == ouch_state::not_logged_in) return vector<char>();
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    return vector<char>(reinterpret_cast<const char*>(&h), reinterpret_cast<const char*>(&h) + sizeof(h));
  }
  return vector<char>();
}

vector<char> ouch_session::constructOrderAccpeted(EnterOrder * eo, const order & o){
  OrderAccepted oa;
  oa.timestamp = get_timestamp();
  oa.token = eo->token;
  oa.side = eo->side;
  oa.qty = eo->qty;
  strncpy(oa.symbol, eo->symbol, sizeof(oa.symbol));
  oa.price = eo->price;
  oa.time_in_force = eo->time_in_force;
  strncpy(oa.firm, eo->firm, sizeof(oa.firm));
  oa.display = eo->display;
  oa.capacity = eo->capacity;
  oa.intermarket_sweep_eligibility = eo->intermarket_sweep_eligibility;
  oa.min_qty = eo->min_qty;
  oa.cross_type = eo->cross_type;
  //TODO: hardcoded BBO weight indicator
  oa.bbo_weight_indicator = '2';
  oa.order_state = static_cast<char>(ouch::OrderState::Live);
  //TODO: order_reference_number
  oa.order_reference_number = o.orderID;
  oa.to_network();
  return vector<char>(reinterpret_cast<const char*>(&oa), reinterpret_cast<const char*>(&oa) + sizeof(oa));
}

vector<char> ouch_session::constructOrderRejected(EnterOrder* eo){
  OrderRejected oj;
  oj.token = eo->token;
  oj.timestamp = get_timestamp();
  oj.reason = 'O'; //reject reason: Other
  oj.to_network();
  return vector<char>(reinterpret_cast<const char*>(&oj), reinterpret_cast<const char*>(&oj) + sizeof(oj));
}
