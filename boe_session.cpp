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
  seq_num = 0;
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
    case static_cast<char>(MsgType::CancelOrder):
      cancelOrder(hdr, len);
      break;
  }
}

void boe_session::cancel_logic(){
  vector<string> done_tokens;
  for (const auto & co : pending_cancel){
    if (active_orders.find(co.token._str_()) == active_orders.end()){
      constructCancelRejected(co.token);
      continue;
    }
    done_tokens.push_back(co.token._str_());
    constructOrderCanceled(co.token);
  }
  pending_cancel.clear();
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void boe_session::cancelOrder(MsgHeader * msg, size_t len){
  CancelOrder* co = reinterpret_cast<CancelOrder*>(msg);
  string t = co->orig_token._str_();
  if (active_orders.find(t) != active_orders.end() and active_orders[t].remaining_qty>0){
    pending_cancel.push_back(Boe_CancelOrderReq(co));
  }
  else constructCancelRejected(co->orig_token);
}

void boe_session::constructOrderCanceled(Token t){
  OrderCanceled oc;
  oc.transaction_time = get_timestamp();
  oc.token = t;
  oc.reason = static_cast<char>(Reason::UserRequested);
  oc.num_bitfields = 0;
  auto packet = vector<char>(reinterpret_cast<char*>(&oc), reinterpret_cast<char*>(&oc)+sizeof(oc));
  pending_out_messages.push_back(packet);
}

void boe_session::constructCancelRejected(Token t){
  CancelRejected cr;
  cr.transaction_time = get_timestamp();
  cr.token = t;
  cr.reason = static_cast<char>(Reason::ClordidDoesntMatchAKnownOrder);
  cr.num_bitfields = 0;
  strcpy(cr.text, "Cannot locate specified order");
  auto packet = vector<char>(reinterpret_cast<char*>(&cr), reinterpret_cast<char*>(&cr)+sizeof(cr));
  pending_out_messages.push_back(packet);
}

void boe_session::enterOrder(MsgHeader * hdr, size_t len){
  NewOrder * no = reinterpret_cast<NewOrder*>(hdr);
  string t = no->token._str_();
  if (active_orders.find(t) != active_orders.end())
    constructOrderRejected(no);
  boe_order new_order = boe_order(no);
  active_orders[t] = new_order;
  constructOrderAccpeted(new_order);
}

void boe_session::constructOrderRejected(NewOrder * no){
  OrderRejected oj;
  oj.token = no->token;
  oj.reason = 'D';
  strcpy(oj.text, "Duplicate CIOrdID");
  auto packet = vector<char>(reinterpret_cast<char*>(&oj), reinterpret_cast<char*>(&oj)+sizeof(oj));
  pending_out_messages.push_back(packet);
}

void boe_session::constructOrderAccpeted(boe_order & new_order){
  OrderAck ack;
  ack.seq_num = ++seq_num;
  ack.transaction_time = get_timestamp();
  ack.order_id = new_order.orderID;
  ack.token = new_order.token;
  ack.num_bitfields = 0;
  auto packet = vector<char>(reinterpret_cast<char*>(&ack), reinterpret_cast<char*>(&ack)+sizeof(ack));
  pending_out_messages.push_back(packet);
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
  free(buf);
}

uint64_t boe_session::get_timestamp(){
  chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
  return ms.count() * 1000;
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
  cancel_logic();
  execution_logic();
}

void boe_session::execution_logic(){
  vector<string> done_tokens;
  for (auto & order_pair : active_orders){
    boe_order & each_order = order_pair.second;
    const string & each_token = order_pair.first;
    if (each_order.still_live()){
      if (order_random_reject()) continue;
      constructOrderExecuted(each_order);
    }
    else{
      finished_orders[each_token] = each_order;
      done_tokens.push_back(each_token);
    }
  }
  for (const auto & each_token : done_tokens)
    active_orders.erase(each_token);
}

void boe_session::constructOrderExecuted(boe_order & curr_order){
  OrderExecution oe;
  oe.seq_num = ++seq_num;
  oe.transaction_time = get_timestamp();
  oe.token = curr_order.token;
  oe.exec_id = rand() * rand();
  uint32_t exe_qty = (2 + rand() % 10) * 100;
  exe_qty = min(exe_qty, curr_order.remaining_qty);
  oe.last_shares = exe_qty;
  curr_order.remaining_qty -= exe_qty;
  oe.last_price = curr_order.price;
  oe.leaves_qty = curr_order.remaining_qty;
  oe.base_liquidity_indicator = 'A';
  oe.sub_liquidity_indicator = 'E';
  strncpy(oe.contra_broker, "BATS", 4);
  auto packet = vector<char>(reinterpret_cast<const char*>(&oe), reinterpret_cast<const char*>(&oe) + sizeof(oe));
  pending_out_messages.push_back(packet);
}
