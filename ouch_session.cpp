#include "ouch_session.h"

void ouch_session::handle_packet(char * packet, size_t len){
  MsgHeader * msg_h = reinterpret_cast<MsgHeader*>(packet);
  switch (msg_h->packet_type){
    case(static_cast<char>(PacketType::LoginRequest)):
      handle_login_request(msg_h, len);
      break;
    case(static_cast<char>(PacketType::LogoutRequest)):
      handle_logout_request(msg_h, len);
      break;
    case(static_cast<char>(PacketType::ClientHeartbeat)):
      handle_client_heartbeat(msg_h, len);
      break;
    case(static_cast<char>(PacketType::UnsequencedData)):
      handle_message(msg_h, len);
      break;
    default:
      return;
  }
}

void ouch_session::handle_message(MsgHeader * packet, size_t len){
  Ouch_MsgHeader * ouch_msg_h = reinterpret_cast<Ouch_MsgHeader*>(packet);
  switch (ouch_msg_h->msg_type) {
    case(static_cast<char>(OutboundMsgType::EnterOrder)):
      enterOrder(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::ReplaceOrder)):
      replaceOrder(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::CancelOrder)):
      cancelOrder(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::ModifyOrder)):
      modifyOrder(ouch_msg_h, len);
      break;
    default:
      return;
  }
}

void ouch_session::replaceOrder(Ouch_MsgHeader * msg, size_t len){
  return;
}

void ouch_session::cancelOrder(Ouch_MsgHeader * msg, size_t len){
  return;
}

void ouch_session::modifyOrder(Ouch_MsgHeader * msg, size_t len){
  return;
}

void ouch_session::market_logic(){
  heartbeat();
  vector<string> done_tokens;
  for (auto & order_pair : LiveOrders){
    order & each_order = order_pair.second;
    const string & each_token = order_pair.first;
    if (each_order.still_live()){
      //randomly mess up execution
      if (rand() % 2) continue;
      execute_order(each_order);
    }
    else{
      DoneOrders[each_token] = each_order;
      done_tokens.push_back(each_token);
    }
  }
  for (const auto & each_token : done_tokens)
    LiveOrders.erase(each_token);
}

void ouch_session::execute_order(order & o){
  uint32_t exe_qty = (2 + rand() % 10) * 100;
  exe_qty = min(exe_qty, o.qty);
  Executed ex;
  ex.timestamp = get_timestamp();
  ex.token = o.token;
  ex.executed_qty = exe_qty;
  ex.execution_price = o.price;
  ex.liquidity_flag = 'R';
  ex.match_number = get_timestamp();
  o.qty -= exe_qty;
  ex.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&ex), reinterpret_cast<const char*>(&ex) + sizeof(ex));
  pending_out_messages.push_back(packet);
}

void ouch_session::enterOrder(Ouch_MsgHeader * msg, size_t len){
  EnterOrder * eo = reinterpret_cast<EnterOrder*>(msg);
  eo->from_network();

  //TODO: randomly reject orders
  if (!(rand() % 5) or (state != ouch_state::logged_in)){
    constructOrderRejected(eo);
    return;
  }
  //this order never seen
  if ((LiveOrders.find(string(eo->token.val)) == LiveOrders.end()) and
      (DoneOrders.find(string(eo->token.val)) == DoneOrders.end())){
    order new_order = order(*eo);
    LiveOrders[string(eo->token.val)] = new_order;
    constructOrderAccpeted(eo, new_order);
    return;
  }
  //ignore enter order
  return;
}

uint64_t ouch_session::get_timestamp(){
  auto curr_time = chrono::high_resolution_clock::now();
  chrono::duration<uint64_t, std::nano> diff = curr_time - start_of_day;
  return diff.count();
}

void ouch_session::handle_login_request(MsgHeader * packet, size_t len){
  LoginRequest * r = reinterpret_cast<LoginRequest*>(packet);
  if (login(r)){
    LoginAccepted la;
    //TODO: session and seq num assignment logic
    strncpy(la.session, "         0", sizeof(la.session));
    strncpy(la.seq_num, "                   0", sizeof(la.seq_num));
    auto packet = vector<char>(reinterpret_cast<const char*>(&la), reinterpret_cast<const char*>(&la) + sizeof(la));
    pending_out_messages.push_back(packet);
  }else{
    LoginRejected lj;
    lj.reason = 'A';
    auto packet =  vector<char>(reinterpret_cast<const char*>(&lj), reinterpret_cast<const char*>(&lj) + sizeof(lj));
    pending_out_messages.push_back(packet);
  }
}

void ouch_session::handle_logout_request(MsgHeader * packet, size_t len){
  state = ouch_state::not_logged_in;
  return;
}

void ouch_session::handle_client_heartbeat(MsgHeader * packet, size_t len){
  last_recv_heartbeat = clock();
  return;
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

void ouch_session::heartbeat(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == ouch_state::not_logged_in) return;
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    auto packet = vector<char>(reinterpret_cast<const char*>(&h), reinterpret_cast<const char*>(&h) + sizeof(h));
    pending_out_messages.push_back(packet);
  }
}

void ouch_session::constructOrderAccpeted(EnterOrder * eo, const order & o){
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
  auto packet = vector<char>(reinterpret_cast<const char*>(&oa), reinterpret_cast<const char*>(&oa) + sizeof(oa));
  pending_out_messages.push_back(packet);
}

void ouch_session::constructOrderRejected(EnterOrder* eo){
  OrderRejected oj;
  oj.token = eo->token;
  oj.timestamp = get_timestamp();
  oj.reason = 'O'; //reject reason: Other
  oj.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&oj), reinterpret_cast<const char*>(&oj) + sizeof(oj));
  pending_out_messages.push_back(packet);
}
