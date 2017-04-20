#include "session.h"
using namespace evt::boe;
using namespace evt;
using namespace evtsim;
using namespace std;

boe_session::boe_session(BehaviorManager * bm){
  _behavior = bm;
  bm->register_status_function(bind(&boe_session::curr_status, this));
  bm->register_cancel_function(bind(&boe_session::cancel_all, this));
  init();
}

boe_session::~boe_session(){
}

string boe_session::curr_status(){
  stringstream ss;
  ss << "[BOE]Active Orders:" << endl;
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

void boe_session::init(){
  seq_num = 0;
  state = SessionState::NotLoggedIn;
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
      enter_order(hdr, len);
      break;
    case static_cast<char>(MsgType::CancelOrder):
      cancel_order(hdr, len);
      break;
    case static_cast<char>(MsgType::ModifyOrder):
      modify_order(hdr, len);
      break;
  }
}

void boe_session::handle_client_heartbeat(MsgHeader* hdr, size_t len){
  last_recv_heartbeat = clock();
}

void boe_session::handle_login_request(MsgHeader* hdr, size_t len){
  LoginRequest * req = reinterpret_cast<LoginRequest*>(hdr);
  if (!_behavior->login())
    construct_login_response(LoginResponseStatus::SessionDisabled, req);
  else if (state != SessionState::NotLoggedIn)
    construct_login_response(LoginResponseStatus::SessionInUse, req);
  else{
    construct_login_response(LoginResponseStatus::Accepted, req);
    state = SessionState::LoggedIn;
  }
}

void boe_session::modify_order(MsgHeader * msg, size_t len){
  ModifyOrder * mo = reinterpret_cast<ModifyOrder*>(msg);
  string t = mo->orig_token._str_();
  if (active_orders.find(t) == active_orders.end() and finished_orders.find(t) == finished_orders.end()){
    construct_modify_rejected(mo->orig_token, Reason::ClordidDoesntMatchAKnownOrder);
    return;
  }
  if (!_behavior->modify_order()){
    construct_modify_rejected(mo->orig_token, Reason::UnforeseenReason);
    return;
  }
  Boe_ModifyOrderReq modify_req = Boe_ModifyOrderReq(mo);
  pending_modify.push_back(modify_req);
}

void boe_session::cancel_order(MsgHeader * msg, size_t len){
  CancelOrder* co = reinterpret_cast<CancelOrder*>(msg);
  string t = co->orig_token._str_();
  if (active_orders.find(t) != active_orders.end() and active_orders[t].remaining_qty > 0){
    pending_cancel.push_back(Boe_CancelOrderReq(co));
  }
  else if (active_orders[t].remaining_qty <= 0 or finished_orders.find(t) != finished_orders.end()){
    construct_cancel_rejected(co->orig_token, Reason::TooLateToCancel);
  }
  else if (active_orders.find(t) == active_orders.end())
    construct_cancel_rejected(co->orig_token, Reason::ClordidDoesntMatchAKnownOrder);
  else if (!_behavior->cancel_order())
    construct_cancel_rejected(co->orig_token, Reason::UnforeseenReason);
}

void boe_session::enter_order(MsgHeader * hdr, size_t len){
  NewOrder * no = reinterpret_cast<NewOrder*>(hdr);
  string t = no->token._str_();
  if (active_orders.find(t) != active_orders.end() or !_behavior->new_order())
    construct_order_rejected(no);
  else{
    Boe_Order new_order = Boe_Order(no);
    active_orders[t] = new_order;
    construct_order_accepted(new_order);
  }
}

void boe_session::market_logic(){
  heartbeat_logic();
  cancel_logic();
  modify_logic();
  execution_logic();
}

void boe_session::execution_logic(){
  vector<string> done_tokens;
  for (auto & order_pair : active_orders){
    Boe_Order & each_order = order_pair.second;
    const string & each_token = order_pair.first;
    if (!_behavior->execution())
      continue;
    if (each_order.still_live())
      construct_order_executed(each_order, _behavior->execution_qty());
    else{
      finished_orders[each_token] = each_order;
      done_tokens.push_back(each_token);
    }
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void boe_session::modify_logic(){
  vector<string> done_tokens;
  for (const auto & mo: pending_modify){
    if (active_orders.find(mo.orig_token._str_()) == active_orders.end())
      continue;
    auto & target_order = active_orders[mo.orig_token._str_()];

    if (target_order.executed_qty >= mo.qty){
      done_tokens.push_back(target_order.token._str_());
      target_order.remaining_qty = 0;
    }
    else target_order.remaining_qty = mo.qty - target_order.executed_qty;
    target_order.side = mo.side;
    target_order.price = mo.price;
    target_order.token = mo.token;
    strncpy(target_order.firm, mo.clearing_firm, 4);
    //change the key in the map
    swap(active_orders[mo.token._str_()], active_orders[mo.orig_token._str_()]);
    active_orders.erase(mo.orig_token._str_());
    construct_order_modified(active_orders[mo.token._str_()]);
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void boe_session::cancel_logic(){
  vector<string> done_tokens;
  for (const auto & co : pending_cancel){
    if (active_orders.find(co.token._str_()) == active_orders.end()){
      construct_cancel_rejected(co.token, Reason::TooLateToCancel);
      continue;
    }
    else if (!_behavior->cancel_order()){
      construct_cancel_rejected(co.token, Reason::UnforeseenReason);
      continue;
    }
    done_tokens.push_back(co.token._str_());
    construct_order_canceled(co.token);
  }
  pending_cancel.clear();
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

int boe_session::cancel_all(){
  int num_canceled = 0;
  for (const auto & it : active_orders){
    const auto & order = it.second;
    if (order.still_alive()){
      construct_order_canceled(order.token);
      num_canceled++;
    }
  }
  active_orders.clear();
  return num_canceled;
}

void boe_session::heartbeat_logic(){
  double second = difftime(time(NULL), last_send_heartbeat);
  if (state == SessionState::NotLoggedIn) return;
  if (second >= 1){
    last_send_heartbeat = time(NULL);
    ServerHeartbeat h;
    auto packet = vector<char>(reinterpret_cast<const char*>(&h), reinterpret_cast<const char*>(&h) + sizeof(h));
    pending_out_messages.push_back(packet);
  }
}

void boe_session::construct_modify_rejected(Token t, Reason r){
  ModifyRejected mr;
  mr.transaction_time = get_timestamp();
  mr.token = t;
  mr.reason = static_cast<char>(r);
  strcpy(mr.text, to_string(r));
  mr.num_bitfields = 0;
  auto packet = vector<char>(reinterpret_cast<char*>(&mr), reinterpret_cast<char*>(&mr)+sizeof(mr));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_order_modified(const Boe_Order & bo){
  OrderModified om;
  //price, OrderQty, LeavesQty
  om.bitfield[0] = 4;
  om.bitfield[1] = 0;
  om.bitfield[2] = 64;
  om.bitfield[3] = 0;
  om.bitfield[4] = 2;
  om.price = bo.price;
  om.leaves_qty = bo.remaining_qty;
  om.qty = bo.remaining_qty + bo.executed_qty;
  om.transaction_time = get_timestamp();
  om.order_id = bo.orderID;
  om.token = bo.token;
  auto packet = vector<char>(reinterpret_cast<char*>(&om), reinterpret_cast<char*>(&om)+sizeof(om));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_order_canceled(Token t){
  OrderCanceled oc;
  oc.transaction_time = get_timestamp();
  oc.token = t;
  oc.reason = static_cast<char>(Reason::UserRequested);
  memset(oc.bitfield, 0, sizeof(oc.bitfield));
  oc.bitfield[4] = 2;
  auto packet = vector<char>(reinterpret_cast<char*>(&oc), reinterpret_cast<char*>(&oc)+sizeof(oc));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_cancel_rejected(Token t, Reason r){
  CancelRejected cr;
  cr.transaction_time = get_timestamp();
  cr.token = t;
  cr.reason = static_cast<char>(r);
  cr.num_bitfields = 0;
  strcpy(cr.text, to_string(r));
  auto packet = vector<char>(reinterpret_cast<char*>(&cr), reinterpret_cast<char*>(&cr)+sizeof(cr));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_order_rejected(NewOrder * no){
  OrderRejected oj;
  oj.token = no->token;
  oj.reason = 'D';
  strcpy(oj.text, "Duplicate CIOrdID");
  auto packet = vector<char>(reinterpret_cast<char*>(&oj), reinterpret_cast<char*>(&oj)+sizeof(oj));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_order_accepted(Boe_Order & new_order){
  OrderAck ack;
  ack.seq_num = ++seq_num;
  ack.transaction_time = get_timestamp();
  ack.order_id = new_order.orderID;
  ack.token = new_order.token;
  ack.num_bitfields = 5;
  memset(ack.bitfield, 0, sizeof(ack.bitfield));
  ack.bitfield[4] = 2;
  ack.leaves_qty = new_order.remaining_qty;
  auto packet = vector<char>(reinterpret_cast<char*>(&ack), reinterpret_cast<char*>(&ack)+sizeof(ack));
  pending_out_messages.push_back(packet);
}

void boe_session::construct_login_response(LoginResponseStatus status, LoginRequest * req){
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
  free(buf);
}

void boe_session::construct_order_executed(Boe_Order & curr_order, int64_t exe_qty/*=-1*/){
  if (!exe_qty) return;
  OrderExecution oe;
  oe.seq_num = ++seq_num;
  oe.transaction_time = get_timestamp();
  oe.token = curr_order.token;
  oe.exec_id = rand() * rand();
  if (exe_qty < 0) exe_qty = (1 + rand() % 10) * 100;
  exe_qty = min(exe_qty, curr_order.remaining_qty);
  oe.last_shares = exe_qty;
  curr_order.remaining_qty -= exe_qty;
  curr_order.executed_qty += exe_qty;
  oe.last_price = curr_order.price;
  oe.leaves_qty = curr_order.remaining_qty;
  oe.base_liquidity_indicator = 'A';
  oe.sub_liquidity_indicator = 'E';
  strncpy(oe.contra_broker, "BATS", 4);
  auto packet = vector<char>(reinterpret_cast<const char*>(&oe), reinterpret_cast<const char*>(&oe) + sizeof(oe));
  pending_out_messages.push_back(packet);
}

uint64_t boe_session::get_timestamp(){
  chrono::nanoseconds ms = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch());
  return ms.count();
}
