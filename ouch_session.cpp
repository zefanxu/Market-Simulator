#include "session.h"
using namespace evt::ouch;
using namespace evt;
using namespace std;
using namespace evtsim;

//SoupBinTCP functions
void ouch_session::handle_login_request(MsgHeader * packet, size_t len){
  if (state != SessionState::NotLoggedIn)
    construct_login_rejected(LoginRejectReason::SessionNotAvailable);
  else if (!_behavior->login())
    construct_login_rejected(LoginRejectReason::NotAuthorized);
  else{
    state = SessionState::LoggedIn;
    construct_login_accepted();
  }
}

void ouch_session::handle_logout_request(MsgHeader * packet, size_t len){
  state = SessionState::NotLoggedIn;
  return;
}

void ouch_session::handle_client_heartbeat(MsgHeader * packet, size_t len){
  last_recv_heartbeat = clock();
  return;
}

void ouch_session::handle_packet(char * packet, size_t len){
  MsgHeader * msg_h = reinterpret_cast<MsgHeader*>(packet);
  if (!validate(msg_h, len)) return;
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

//OUCH functions
string ouch_session::curr_status(){
  stringstream ss;
  if (!active_orders.size()) return "[OUCH]No active order";
  ss << "[OUCH]Active Order(s):" << endl;
  for (const auto & it : active_orders){
    const auto & order = it.second;
    ss << "[OrderID: " << order.orderID << "]" << endl;
    ss << "Symbol: " << string(order.symbol, sizeof(order.symbol)) << endl;
    ss << "Side: " << order.side << " Price: " << order.price << endl;
    ss << "Executed qty: " << order.executed_qty << endl;
    ss << "Remaining qty: " << order.remaining_qty << endl;
  }
  return ss.str();
}

void ouch_session::handle_message(MsgHeader * packet, size_t len){
  Ouch_MsgHeader * ouch_msg_h = reinterpret_cast<Ouch_MsgHeader*>(packet);
  switch (ouch_msg_h->msg_type) {
    case(static_cast<char>(OutboundMsgType::EnterOrder)):
      enter_order(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::ReplaceOrder)):
      replace_order(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::CancelOrder)):
      cancel_order(ouch_msg_h, len);
      break;
    case(static_cast<char>(OutboundMsgType::ModifyOrder)):
      modify_order(ouch_msg_h, len);
      break;
    default:
      return;
  }
}

void ouch_session::enter_order(Ouch_MsgHeader * msg, size_t len){
  EnterOrder * eo = reinterpret_cast<EnterOrder*>(msg);
  eo->from_network();
  if (eo->qty > MAX_SHARES)
    construct_order_rejected(RejectedReason::ExceededConfiguredSafety, eo->token);
  else if ((state != SessionState::LoggedIn) or !_behavior->new_order())
    construct_order_rejected(RejectedReason::Other, eo->token);
  else if((active_orders.find(eo->token._str_()) == active_orders.end()) and
      (finished_orders.find(eo->token._str_()) == finished_orders.end())){
    Ouch_Order new_order = Ouch_Order(eo);
    active_orders[eo->token._str_()] = new_order;
    construct_order_accepted(new_order);
  }
}

void ouch_session::modify_order(Ouch_MsgHeader * msg, size_t len){
  ModifyOrder * mo_msg = reinterpret_cast<ModifyOrder*>(msg);
  mo_msg -> from_network();
  if (active_orders.find(mo_msg->token._str_()) == active_orders.end())
    return;
  if (!_behavior->modify_order())
    return;
  Ouch_ModifyOrderReq mo = Ouch_ModifyOrderReq(mo_msg);
  pending_modify.push_back(mo);
}

void ouch_session::replace_order(Ouch_MsgHeader * msg, size_t len){
  if (!_behavior->replace_order()) return;
  ReplaceOrder * ro_msg = reinterpret_cast<ReplaceOrder*>(msg);
  ro_msg->from_network();
  Ouch_ReplaceOrderReq ro = Ouch_ReplaceOrderReq(ro_msg);
  pending_replace.push_back(ro);
  return;
}

void ouch_session::cancel_order(Ouch_MsgHeader * msg, size_t len){
  CancelOrder * co_msg = reinterpret_cast<CancelOrder*>(msg);
  co_msg->from_network();
  if (active_orders.find(co_msg->token._str_()) == active_orders.end())
    return;
  if (!_behavior->cancel_order())
    return;
  Ouch_CancelOrderReq co = Ouch_CancelOrderReq(co_msg);
  pending_cancel.push_back(co);
}

void session::market_logic(){
  heartbeat_logic();
  cancel_logic();
  modify_logic();
  replace_logic();
  execution_logic();
}

void session::set_logger(evtsim::Logger * l){
  this->l = l;
}

void ouch_session::replace_logic(){
  vector<string> done_tokens;
  for (const auto & ro : pending_replace){
    const auto t = ro.existing_token._str_();
    auto & target_order = active_orders[t];
    if (active_orders.find(t) == active_orders.end())
      continue;
    if (ro.qty > MAX_SHARES or ro.qty <= target_order.executed_qty){
      done_tokens.push_back(t);
      construct_order_canceled(active_orders[t].remaining_qty, CancelReason::SystemCancel, ro.existing_token);
      continue;
    }
    target_order.token = ro.new_token;
    target_order.remaining_qty = ro.qty - target_order.executed_qty;
    target_order.time_in_force = ro.time_in_force;
    target_order.remain_time_in_force = ro.time_in_force;
    target_order.price = ro.price;
    target_order.intermarket_sweep_eligibility = ro.intermarket_sweep_eligibility;
    construct_order_replaced(ro, target_order);
    //update the key in the map
    swap(active_orders[ro.existing_token._str_()], active_orders[ro.new_token._str_()]);
    active_orders.erase(ro.existing_token._str_());
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
  pending_replace.clear();
}


void ouch_session::modify_logic(){
  vector<string> done_tokens;
  for (const auto & mo: pending_modify){
    if (active_orders.find(mo.token._str_()) == active_orders.end())
      continue;
    auto & target_order = active_orders[mo.token._str_()];
    char new_side = mo.new_side;
    switch ((target_order.side)) {
      case (Side::Sell):
        if (new_side != Side::SellShort and new_side != Side::SellShortExempt)
          continue;
      case (Side::SellShortExempt):
        if (new_side != Side::SellShort and new_side != Side::Sell)
          continue;
      case (Side::SellShort):
        if (new_side != Side::SellShortExempt and new_side != Side::Sell)
          continue;
    }
    if (target_order.executed_qty >= mo.req_qty){
      done_tokens.push_back(target_order.token._str_());
      construct_order_modified(0, mo);
      continue;
    }
    target_order.side = mo.new_side;
    target_order.remaining_qty = mo.req_qty - target_order.executed_qty;
    construct_order_modified(target_order.remaining_qty, mo);
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

int ouch_session::cancel_all(){
  int num_canceled = 0;
  for (auto & it : active_orders){
    auto & order = it.second;
    if (order.still_active()){
      construct_order_canceled(order.remaining_qty, CancelReason::SystemCancel, order.token);
      num_canceled++;
    }
  }
  active_orders.clear();
  return num_canceled;
}

void ouch_session::construct_cancel_rejected(Token t){
  CancelReject cr;
  cr.timestamp = get_timestamp();
  cr.token = t;
  cr.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&cr), reinterpret_cast<const char*>(&cr) + sizeof(cr));
  pending_out_messages.push_back(packet);
}

void ouch_session::cancel_logic(){
  vector<string> done_tokens;
  for (const auto & co : pending_cancel){
      if (active_orders.find(co.token._str_()) == active_orders.end() or !_behavior->cancel_order()){
        construct_cancel_rejected(co.token);
        continue;
      }
      uint32_t curr_qty = active_orders[co.token._str_()].remaining_qty;
      uint32_t dec_qty = curr_qty;
      if (!co.qty)
        done_tokens.push_back(co.token._str_());
      else{
        if (co.qty >= curr_qty) continue;
        dec_qty = curr_qty - co.qty;
        active_orders[co.token._str_()].remaining_qty = co.qty;
      }
      construct_order_canceled(dec_qty, CancelReason::UserRequested, co.token);
  }
  pending_cancel.clear();
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void ouch_session::execution_logic(){
  vector<string> done_tokens;
  for (auto & it : active_orders){
    Ouch_Order & each_order = it.second;
    const string & each_token = it.first;
    if (!_behavior->execution())
      continue;
    if (each_order.expired()){
      construct_order_canceled(each_order.remaining_qty, CancelReason::TimeOut, each_order.token);
      done_tokens.push_back(each_token);
      continue;
    }
    //IOC orders
    if (!each_order.time_in_force){
      if (each_order.remaining_qty)
        construct_order_canceled(each_order.remaining_qty, CancelReason::IOC, each_order.token);
      done_tokens.push_back(each_token);
      continue;
    }
    else if (each_order.still_active())
      construct_order_executed(each_order);
    else{
      finished_orders[each_token] = each_order;
      done_tokens.push_back(each_token);
    }
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void ouch_session::heartbeat_logic(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == SessionState::NotLoggedIn) return;
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    auto packet = vector<char>(reinterpret_cast<const char*>(&h), reinterpret_cast<const char*>(&h) + sizeof(h));
    pending_out_messages.push_back(packet);
  }
}

uint64_t ouch_session::get_timestamp(){
  auto curr_time = chrono::high_resolution_clock::now();
  chrono::duration<uint64_t, std::nano> diff = curr_time - start_of_day;
  return diff.count();
}

ouch_session::ouch_session(BehaviorManager * bm){
  _behavior = bm;
  bm->register_status_function(bind(&ouch_session::curr_status, this));
  bm->register_cancel_function(bind(&ouch_session::cancel_all, this));
  init();
}

ouch_session::~ouch_session(){
}

void ouch_session::init(){
  state = SessionState::NotLoggedIn;
  time_t curr_time = time(NULL);
  last_send_heartbeat = curr_time;
  last_recv_heartbeat = curr_time;

  l = nullptr;

  //get the start of the day time
  time_t t1 = time(NULL);
  struct tm tms;
  localtime_r(&t1, &tms);
  tms.tm_hour = 0;
  tms.tm_min = 0;
  tms.tm_sec = 0;
  t1 = mktime(&tms);
  start_of_day = chrono::system_clock::from_time_t(t1);
}

void ouch_session::construct_order_accepted(const Ouch_Order & o){
  OrderAccepted oa;
  oa.timestamp = get_timestamp();
  oa.token = o.token;
  oa.side = o.side;
  oa.qty = o.remaining_qty;
  strncpy(oa.symbol, o.symbol, sizeof(oa.symbol));
  oa.price = o.price;
  oa.time_in_force = o.time_in_force;
  strncpy(oa.firm, o.firm, sizeof(oa.firm));
  oa.display = o.display;
  oa.capacity = o.capacity;
  oa.intermarket_sweep_eligibility = o.intermarket_sweep_eligibility;
  oa.min_qty = o.min_qty;
  oa.cross_type = o.cross_type;
  //TODO: hardcoded BBO weight indicator
  oa.bbo_weight_indicator = '2';
  oa.order_state = static_cast<char>(ouch::OrderState::Live);
  //TODO: order_reference_number
  oa.order_reference_number = o.orderID;
  oa.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&oa), reinterpret_cast<const char*>(&oa) + sizeof(oa));
  pending_out_messages.push_back(packet);
}

void ouch_session::construct_order_rejected(RejectedReason reason, Token t){
  OrderRejected oj;
  oj.token = t;
  oj.timestamp = get_timestamp();
  oj.reason = static_cast<char>(reason);
  oj.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&oj), reinterpret_cast<const char*>(&oj) + sizeof(oj));
  pending_out_messages.push_back(packet);
}

void ouch_session::construct_order_canceled(uint32_t dec_qty, CancelReason reason, Token t){
  OrderCanceled oc;
  oc.timestamp = get_timestamp();
  oc.token = t;
  oc.decrement_qty = dec_qty;
  oc.reason = static_cast<char>(reason);
  oc.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&oc), reinterpret_cast<const char*>(&oc) + sizeof(oc));
  pending_out_messages.push_back(packet);
}

void ouch_session::construct_order_modified(uint32_t remaining_qty, const Ouch_ModifyOrderReq & mo){
  OrderModified m;
  m.timestamp = get_timestamp();
  m.token = mo.token;
  m.side = mo.new_side;
  m.shares = remaining_qty;
  m.to_network();
  auto packet = vector<char>(reinterpret_cast<const char*>(&m), reinterpret_cast<const char*>(&m) + sizeof(m));
  pending_out_messages.push_back(packet);
}

void ouch_session::construct_order_executed(Ouch_Order & o, int64_t exe_qty/*=-1*/){
  if (!exe_qty) return;
  if (exe_qty < 0){
    exe_qty = (1+rand() % 10) * 100; //execute at least 100 shares
  }
  exe_qty = min(exe_qty, o.remaining_qty);
  if (o.min_qty and exe_qty > o.min_qty)
    return;
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

void ouch_session::construct_order_replaced(const Ouch_ReplaceOrderReq & ro, const Ouch_Order & new_order){
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

bool ouch_session::validate(MsgHeader* msg_h, size_t len){
  if (!l) throw runtime_error("evtsim::Logger has not been set");
  if (big_to_native(msg_h->length) != (len-2))
    l->write_warning("message length mismatch: "+outbound_to_string(msg_h));
  switch (msg_h->packet_type){
    case(static_cast<char>(PacketType::LoginRequest)):
      return validate_login_request(msg_h, len);
    case(static_cast<char>(PacketType::LogoutRequest)):
      return validate_logout_request(msg_h, len);
    case(static_cast<char>(PacketType::ClientHeartbeat)):
      return validate_client_heartbeat(msg_h, len);
    case(static_cast<char>(PacketType::UnsequencedData)):
      auto ouch_msg_h = reinterpret_cast<Ouch_MsgHeader*>(msg_h);
      switch (ouch_msg_h->msg_type) {
        case(static_cast<char>(OutboundMsgType::EnterOrder)):
          return validate_enter_order(msg_h, len);
        case(static_cast<char>(OutboundMsgType::ReplaceOrder)):
          return validate_replace_order(msg_h, len);
        case(static_cast<char>(OutboundMsgType::CancelOrder)):
          return validate_cancel_order(msg_h, len);
        case(static_cast<char>(OutboundMsgType::ModifyOrder)):
          return validate_modify_order(msg_h, len);
      }
  }
  l->write_warning("unsupported message type: " + outbound_to_string(msg_h));
  return false;
}

bool ouch_session::validate_replace_order(MsgHeader * packet, size_t len){
  if (big_to_native(packet->length) != (sizeof(ReplaceOrder)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(packet));
    return false;
  }
  ReplaceOrder ro = *(reinterpret_cast<ReplaceOrder*>(packet));
  ro.from_network();
  if (ro.qty <= 0 or ro.qty > 1000000){
    l->write_warning("invalide ReplaceOrder qty: "+outbound_to_string(packet));
    return false;
  }
  if (ro.display != 'A' and ro.display != 'Y' and ro.display != 'N' and ro.display != 'P'
      and ro.display != 'I' and ro.display != 'M' and ro.display != 'W' and ro.display != 'L'
      and ro.display != 'O' and ro.display != 'T' and ro.display != 'Q'){
        l->write_warning("invalide ReplaceOrder display: "+outbound_to_string(packet));
        return false;
  }
  if (ro.intermarket_sweep_eligibility != 'Y' and ro.intermarket_sweep_eligibility != 'N'
      and ro.intermarket_sweep_eligibility != 'y'){
        l->write_warning("invalide ReplaceOrder intermarket_sweep_eligibility: "+outbound_to_string(packet));
        return false;
  }
  if (ro.price > 1999999900 and ro.price != 2147483647){
    l->write_warning("invalide ReplaceOrder price: "+outbound_to_string(packet));
    return false;
  }
  return true;
}

bool ouch_session::validate_enter_order(MsgHeader * packet, size_t len){
  if (big_to_native(packet->length) != (sizeof(EnterOrder)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(packet));
    return false;
  }
  EnterOrder eo = *(reinterpret_cast<EnterOrder*>(packet));
  eo.from_network();
  if (eo.side != 'B' and eo.side != 'S' and eo.side != 'T' and eo.side != 'E'){
    l->write_warning("invalide EnterOrder side: "+outbound_to_string(packet));
    return false;
  }
  if (eo.price > 1999999900 and eo.price != 2147483647){
    l->write_warning("invalide EnterOrder price: "+outbound_to_string(packet));
    return false;
  }
  if (eo.qty <= 0 or eo.qty > 1000000){
    l->write_warning("invalide EnterOrder qty: "+outbound_to_string(packet));
    return false;
  }
  if (eo.display != 'A' and eo.display != 'Y' and eo.display != 'N' and eo.display != 'P'
      and eo.display != 'I' and eo.display != 'M' and eo.display != 'W' and eo.display != 'L'
      and eo.display != 'O' and eo.display != 'T' and eo.display != 'Q'){
        l->write_warning("invalide EnterOrder display: "+outbound_to_string(packet));
        return false;
  }
  if (eo.intermarket_sweep_eligibility != 'Y' and eo.intermarket_sweep_eligibility != 'N'
      and eo.intermarket_sweep_eligibility != 'y'){
        l->write_warning("invalide EnterOrder intermarket_sweep_eligibility: "+outbound_to_string(packet));
        return false;
  }
  if (eo.cross_type != 'N' and eo.cross_type != 'O' and eo.cross_type != 'C' and eo.cross_type != 'H'
      and eo.cross_type != 'S' and eo.cross_type != 'R'){
        l->write_warning("invalide EnterOrder cross_type: "+outbound_to_string(packet));
        return false;
  }
  if (eo.customer_type != ' ' and eo.customer_type != 'R' and eo.customer_type != 'N'){
    l->write_warning("invalide EnterOrder customer_type: "+outbound_to_string(packet));
    return false;
  }
  return true;
}

bool ouch_session::validate_cancel_order(MsgHeader* packet, size_t len){
  if (big_to_native(packet->length) != (sizeof(CancelOrder)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(packet));
    return false;
  }
  CancelOrder co = *(reinterpret_cast<CancelOrder*>(packet));
  co.from_network();
  if (co.qty > 1000000){
    l->write_warning("invalide CancelOrder qty: "+outbound_to_string(packet));
    return false;
  }
  return true;
}

bool ouch_session::validate_modify_order(MsgHeader* packet, size_t len){
  if (big_to_native(packet->length) != (sizeof(ModifyOrder)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(packet));
    return false;
  }
  ModifyOrder mo = *(reinterpret_cast<ModifyOrder*>(packet));
  mo.from_network();
  if (mo.qty > 1000000){
    l->write_warning("invalide ModifyOrder qty: "+outbound_to_string(packet));
    return false;
  }
  return true;
}

bool ouch_session::validate_client_heartbeat(MsgHeader* msg_h, size_t len){
  if (big_to_native(msg_h->length) != (sizeof(ClientHeartbeat)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(msg_h));
    return false;
  }
  return true;
}

bool ouch_session::validate_logout_request(MsgHeader* msg_h, size_t len){
  if (big_to_native(msg_h->length) != (sizeof(LogoutRequest)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(msg_h));
    return false;
  }
  return true;
}

bool ouch_session::validate_login_request(MsgHeader* msg_h, size_t len){
  if (big_to_native(msg_h->length) != (sizeof(LoginRequest)-2)){
    l->write_warning("message length mismatch: "+outbound_to_string(msg_h));
    return false;
  }
  LoginRequest lr = *(reinterpret_cast<LoginRequest*>(msg_h));
  string seq_num = string(lr.requested_seq_num, sizeof(lr.requested_seq_num));
  regex re = regex("([[:space:]]*)([[:digit:]]+)");
  if (!regex_match(seq_num, re)){
    l->write_warning("invalid login request seq_num: " + outbound_to_string(msg_h));
    return false;
  }
  return true;
}

void ouch_session::construct_login_accepted(unsigned int session_num, unsigned int seq_num){
  LoginAccepted la;
  //left padded with space
  memset(la.session, ' ', sizeof(la.session));
  memset(la.seq_num, ' ', sizeof(la.seq_num));
  string session_str = to_string(session_num);
  string seq_str = to_string(seq_num);
  assert(session_str.size() <= sizeof(la.session));
  assert(seq_str.size() <= sizeof(la.seq_num));
  strncpy(la.session + (sizeof(la.session)-session_str.size()), session_str.c_str(), session_str.size());
  strncpy(la.seq_num + (sizeof(la.seq_num)-seq_str.size()), seq_str.c_str(), seq_str.size());
  auto packet = vector<char>(reinterpret_cast<const char*>(&la), reinterpret_cast<const char*>(&la) + sizeof(la));
  pending_out_messages.push_back(packet);
}

void ouch_session::construct_login_rejected(LoginRejectReason r){
  LoginRejected lj;
  lj.reason = static_cast<char>(r);
  auto packet =  vector<char>(reinterpret_cast<const char*>(&lj), reinterpret_cast<const char*>(&lj) + sizeof(lj));
  pending_out_messages.push_back(packet);
}
