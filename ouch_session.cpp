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
  ReplaceOrder * ro_msg = reinterpret_cast<ReplaceOrder*>(msg);
  ro_msg->from_network();
  replace_order ro = replace_order(ro_msg);
  PendingReplace[ro_msg->existing_token._str_()] = ro;
  return;
}

void ouch_session::cancelOrder(Ouch_MsgHeader * msg, size_t len){
  CancelOrder * co_msg = reinterpret_cast<CancelOrder*>(msg);
  co_msg->from_network();
  if (LiveOrders.find(co_msg->token._str_()) == LiveOrders.end())
    return;
  if (PendingCancel.find(co_msg->token._str_()) != PendingCancel.end())
    return;
  cancel_order co = cancel_order(co_msg);
  PendingCancel[co_msg->token._str_()] = co;
}

void ouch_session::replace_logic(){
  vector<string> done_tokens;
  for (const auto & replace_order_pair : PendingReplace){
    const auto & ro = replace_order_pair.second;
    const auto t = ro.existing_token._str_();
    if (LiveOrders.find(t) == LiveOrders.end())
      continue;
    if (ro.qty > 1000000){
      done_tokens.push_back(t);
      constructOrderCanceled(LiveOrders[t].remaining_qty, 'Z', ro.existing_token);
      continue;
    }
    auto & target_order = LiveOrders[t];
    target_order.token = ro.new_token;
    target_order.remaining_qty = ro.qty;
    target_order.time_in_force = ro.time_in_force;
    target_order.remain_time_in_force = ro.time_in_force;
    target_order.price = ro.price;
    target_order.intermarket_sweep_eligibility = ro.intermarket_sweep_eligibility;
    constructOrderReplaced(ro, target_order);
  }
  for (const auto & each_token : done_tokens)
    LiveOrders.erase(each_token);
  PendingReplace.clear();
}

void ouch_session::constructOrderReplaced(const replace_order & ro, const order & new_order){
  OrderReplaced _or;
  _or.timestamp = get_timestamp();
  _or.token = new_order.token;
  _or.side = new_order.side;
  _or.qty = new_order.remaining_qty;
  strncpy(_or.symbol, new_order.symbol, sizeof(_or.symbol));
  _or.price = new_order.price;
  _or.time_in_force = new_order.time_in_force;
  strncpy(_or.firm, new_order.firm, sizeof(_or.firm));
  _or.capacity = new_order.capacity;
  _or.order_reference_number = new_order.orderID;
  _or.bbo_weight_indicator = '2';
  _or.orig_token = ro.existing_token;
  _or.order_state = static_cast<char>(ouch::OrderState::Live);
  _or.min_qty = new_order.min_qty;
  _or.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&_or), reinterpret_cast<const char*>(&_or)+sizeof(_or));
  pending_out_messages.push_back(packet);
}

void ouch_session::modifyOrder(Ouch_MsgHeader * msg, size_t len){
  ModifyOrder * mo_msg = reinterpret_cast<ModifyOrder*>(msg);
  mo_msg -> from_network();
  if (LiveOrders.find(mo_msg->token._str_()) == LiveOrders.end())
    return;
  modify_order mo = modify_order(mo_msg);
  PendingModify[mo_msg->token._str_()] = mo;
  return;
}

void ouch_session::modify_logic(){
  vector<string> done_tokens;
  for (const auto & cancel_order_pair : PendingModify){
    const modify_order & mo = cancel_order_pair.second;
    if (LiveOrders.find(mo.token._str_()) == LiveOrders.end())
      continue;
    auto & target_order = LiveOrders[mo.token._str_()];
    char new_side = mo.new_side;
    switch (target_order.side) {
      case ('S'):
        if (new_side != 'T' and new_side != 'E')
          continue;
      case ('E'):
        if (new_side != 'T' and new_side != 'S')
          continue;
      case ('T'):
        if (new_side != 'E' and new_side != 'S')
          continue;
    }
    if (target_order.executed_qty >= mo.req_qty){
      done_tokens.push_back(target_order.token._str_());
      constructOrderModified(0, mo);
      continue;
    }
    target_order.side = mo.new_side;
    target_order.remaining_qty = mo.req_qty - target_order.executed_qty;
    constructOrderModified(target_order.remaining_qty, mo);
  }
  for (const auto & each_token : done_tokens)
    LiveOrders.erase(each_token);
}

void ouch_session::cancel_logic(){
  vector<string> done_tokens;
  for (const auto & cancel_order_pair : PendingCancel){
      const cancel_order & co = cancel_order_pair.second;
      if (LiveOrders.find(co.token._str_()) == LiveOrders.end())
        continue;
      uint32_t curr_qty = LiveOrders[co.token._str_()].remaining_qty;
      uint32_t dec_qty = curr_qty;
      if (!co.qty)
        done_tokens.push_back(co.token._str_());
      else{
        if (co.qty >= curr_qty) continue;
        dec_qty = curr_qty - co.qty;
        LiveOrders[co.token._str_()].remaining_qty = co.qty;
      }
      constructOrderCanceled(dec_qty, 'U', co.token);
  }
  PendingCancel.clear();
  for (const auto & each_token : done_tokens)
    LiveOrders.erase(each_token);
}

void ouch_session::execution_logic(){
  vector<string> done_tokens;
  for (auto & order_pair : LiveOrders){
    order & each_order = order_pair.second;
    const string & each_token = order_pair.first;
    if (each_order.expired()){
      constructOrderCanceled(each_order.remaining_qty, 'T', each_order.token);
      done_tokens.push_back(each_token);
      continue;
    }
    if (each_order.still_live()){
      if (rand() % 2) continue;
      constructOrderExecuted(each_order);
    }
    else{
      DoneOrders[each_token] = each_order;
      done_tokens.push_back(each_token);
    }
  }
  for (const auto & each_token : done_tokens)
    LiveOrders.erase(each_token);
}

void ouch_session::market_logic(){
  heartbeat_logic();
  cancel_logic();
  replace_logic();
  execution_logic();
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
  if ((LiveOrders.find(eo->token._str_()) == LiveOrders.end()) and
      (DoneOrders.find(eo->token._str_()) == DoneOrders.end())){
    order new_order = order(eo);
    LiveOrders[eo->token._str_()] = new_order;
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

void ouch_session::heartbeat_logic(){
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

void ouch_session::constructOrderCanceled(uint32_t dec_qty, char reason, Token t){
  OrderCanceled oc;
  oc.timestamp = get_timestamp();
  oc.token = t;
  oc.decrement_qty = dec_qty;
  oc.reason = reason;
  oc.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&oc), reinterpret_cast<const char*>(&oc) + sizeof(oc));
  pending_out_messages.push_back(packet);
}

void ouch_session::constructOrderModified(uint32_t remaining_qty, const modify_order & mo){
  OrderModified m;
  m.timestamp = get_timestamp();
  m.token = mo.token;
  m.side = mo.new_side;
  m.shares = remaining_qty;
  m.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&m), reinterpret_cast<const char*>(&m) + sizeof(m));
  pending_out_messages.push_back(packet);
}

void ouch_session::constructOrderExecuted(order & o){
  uint32_t exe_qty = (2 + rand() % 10) * 100;
  exe_qty = min(exe_qty, o.remaining_qty);
  Executed ex;
  ex.timestamp = get_timestamp();
  ex.token = o.token;
  ex.executed_qty = exe_qty;
  ex.execution_price = o.price;
  ex.liquidity_flag = 'R';
  ex.match_number = get_timestamp();
  o.remaining_qty -= exe_qty;
  o.executed_qty += exe_qty;
  ex.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&ex), reinterpret_cast<const char*>(&ex) + sizeof(ex));
  pending_out_messages.push_back(packet);
}
