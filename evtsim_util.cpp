#include "evtsim_util.h"

string parse_packet(char * packet, size_t len){
  MsgHeader * msg_h = reinterpret_cast<MsgHeader*>(packet);
  cout << outbound_to_string(msg_h) << endl;
  switch (msg_h->packet_type){
    case('L'):
      return parse_login_request(msg_h, len);
    default:
      return string();
  }
}

string parse_login_request(MsgHeader * packet, size_t len){
  LoginRequest * r = reinterpret_cast<LoginRequest*>(packet);
  char username[] = "S155T1";
  char password[] = "testonly  ";
  if (strncmp(r->username, username, sizeof(username)) and strncmp(r->password, password, sizeof(password))){
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

int to_string(const LoginRequest& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "LoginRequest(length=%hu,packet_type=%c,username=%s,password=%s,requested_session=%s,requested_seq_num=%s)",
                big_to_native(m.length), m.packet_type,
                std::string(m.username, sizeof(m.username)).c_str(),
                std::string(m.password, sizeof(m.password)).c_str(),
                std::string(m.requested_session, sizeof(m.requested_session)).c_str(),
                std::string(m.requested_seq_num, sizeof(m.requested_seq_num)).c_str());
  }

int to_string(const ClientHeartbeat& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "ClientHeartbeat(length=%hu,packet_type=%c)", big_to_native(m.length),
                m.packet_type);
}
int to_string(const EnterOrder& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
              "EnterOrder(length=%hu,packet_type=%c,msg_type=%c,clordid=%s,side=%c,qty=%u,symbol=%s,price=%d,time_in_force=%u,firm=%s,display=%c,capacity=%c,intermarket_sweep_eligibility=%c,min_qty=%u,cross_type=%c,customer_type=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, m.clordid.str().c_str(),
                m.side, big_to_native(m.qty), std::string(m.symbol, sizeof(m.symbol)).c_str(),
                big_to_native(m.price), big_to_native(m.time_in_force),
                std::string(m.firm, sizeof(m.firm)).c_str(), m.display, m.capacity,
                  m.intermarket_sweep_eligibility, big_to_native(m.min_qty), m.cross_type,
                  m.customer_type);
  }

int to_string(const CancelOrder& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "CancelOrder(length=%hu,packet_type=%c,msg_type=%c,clordid=%s,qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, m.clordid.str().c_str(),
                big_to_native(m.qty));
}
int to_string(const ModifyOrder& m, char* buf, size_t len) {
return std::snprintf(buf, len, "ModifyOrder(length=%hu,packet_type=%c,msg_type=%c,clordid=%s,side=%c,qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, m.clordid.str().c_str(),
                m.side, big_to_native(m.qty));
}
int to_string(const ReplaceOrder& m, char* buf, size_t len) {
return std::snprintf(buf, len, "ReplaceOrder(length=%hu,packet_type=%c,msg_type=%c,existing_clordid=%s,clordid=%s,qty=%u,price=%d,time_in_force=%u,display=%c,intermarket_sweep_eligibility=%c,min_qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type,
                m.existing_clordid.str().c_str(), m.clordid.str().c_str(), big_to_native(m.qty),
                big_to_native(m.price), big_to_native(m.time_in_force), m.display,
                m.intermarket_sweep_eligibility, m.min_qty);
}

int to_string(const LoginAccepted& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "LoginAccepted(length=%hu,packet_type=%c,session=%s,seq_num=%s)",
                big_to_native(m.length), m.packet_type,
                std::string(m.session, sizeof(m.session)).c_str(),
                std::string(m.seq_num, sizeof(m.seq_num)).c_str());
}
int to_string(const LoginRejected& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "LoginRejected(length=%hu,packet_type=%c,reason=%c)",
                big_to_native(m.length), m.packet_type, m.reason);
}
nt to_string(const OrderAccepted& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
              "OrderAccepted(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,qty=%u,symbol=%s,price=%d,time_in_force=%u,firm=%s,display=%c,order_reference_number=%lu,capacity=%c,intermarket_sweep_eligibility=%c,min_qty=%u,cross_type=%c,order_state=%c,bbo_weight_indicator=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), m.side, big_to_native(m.qty),
                std::string(m.symbol, sizeof(m.symbol)).c_str(), big_to_native(m.price),
                big_to_native(m.time_in_force), std::string(m.firm, sizeof(m.firm)).c_str(),
                  m.display, big_to_native(m.order_reference_number), m.capacity,
                  m.intermarket_sweep_eligibility, big_to_native(m.min_qty), m.cross_type,
                  m.order_state, m.bbo_weight_indicator);
  }

int to_string(const OrderRejected& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderRejected(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), m.reason);
}
int to_string(const ServerHeartbeat& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "ServertHeartbeat(length=%hu,packet_type=%c)", big_to_native(m.length),
                m.packet_type);
}
nt to_string(const EndOfSession& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "EndOfSession(length=%hu,packet_type=%c)", big_to_native(m.length),
              m.packet_type);
}
int to_string(const AiqCanceled& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
              "AiqCanceled(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,decrement_qty=%u,reason=%c,qty_prevented_from_trading=%u,execution_price=%u,liquidity_flag=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.decrement_qty), m.reason,
                big_to_native(m.qty_prevented_from_trading),
                big_to_native(m.execution_price), m.liquidity_flag);
  }

int to_string(const CancelReject& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "CancelReject(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str());
}
int to_string(const Executed& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "Executed(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_qty=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.executed_qty),
                big_to_native(m.execution_price), m.liquidity_flag,
                big_to_native(m.match_number));
  }

int to_string(const ExecutedWithRefPrice& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "ExecutedWithRefPrice(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_qty=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu,reference_price=%u,reference_price_type=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.executed_qty),
                big_to_native(m.execution_price), m.liquidity_flag,
                big_to_native(m.match_number), big_to_native(m.reference_price),
                  m.reference_price_type);
  }

int to_string(const OrderCanceled& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderCanceled(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,decrement_qty=%u,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.decrement_qty), m.reason);
}
int to_string(const OrderModified& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderModified(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,shares=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), m.side, big_to_native(m.shares));
}
int to_string(const OrderReplaced& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderModified(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,qty=%u,symbol=%s,price=%d,time_in_force=%u,firm=%s,display=%c,order_reference_number=%lu,capacity=%c,intermarket_sweep_eligibility=%c,min_qty=%u,cross_type=%c,order_state=%c,orig_clordid=%s,bbo_weight_indicator=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), m.side, big_to_native(m.qty),
                std::string(m.symbol, sizeof(m.symbol)).c_str(), big_to_native(m.price),
                big_to_native(m.time_in_force), std::string(m.firm, sizeof(m.firm)).c_str(),
                  m.display, big_to_native(m.order_reference_number), m.capacity,
                  m.intermarket_sweep_eligibility, big_to_native(m.min_qty), m.cross_type,
                  m.order_state, m.orig_clordid.str().c_str(), m.bbo_weight_indicator);
  }

int to_string(const SystemEvent& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "SystemEvent(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,event_code=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.event_code);
}
int to_string(const BrokenTrade& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "BrokenTrade(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,match_number=%lu,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.match_number), m.reason);
}
int to_string(const CancelPending& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "BrokenTrade(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str());
}
int to_string(const PriorityUpdate& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "PriorityUpdate(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str());
}
int to_string(const TradeCorrection& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "TradeCorrection(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_shares=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str(), big_to_native(m.executed_shares),
                big_to_native(m.execution_price), m.liquidity_flag, big_to_native(m.match_number),
                m.reason);
  }

int to_string(const TradeNow& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "TradeNow(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.clordid.str().c_str());
}


const std::string
inbound_to_string(const MsgHeader* hdr) {

  char buf[4096];
  size_t ret_len = 4096;

  switch(static_cast<PacketType>(hdr->packet_type)) {
  case PacketType::LoginAccepted:
    ret_len = to_string((const LoginAccepted&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::ServerHeartbeat:
    ret_len = to_string((const ServerHeartbeat&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::Debug:
    std::strcpy(buf, "+++ Debug...");
    break;

  case PacketType::EndOfSession:
    ret_len = to_string((const EndOfSession&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::LoginRejected:
    ret_len = to_string((const LoginRejected&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::SequencedData: {
    char msg_type = *((const char*)hdr + sizeof(MsgHeader));

    switch(static_cast<InboundMsgType>(msg_type)) {

    case InboundMsgType::OrderAccepted:
      ret_len = to_string((const OrderAccepted&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::OrderRejected:
      ret_len = to_string((const OrderRejected&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::AiqCanceled:
      ret_len = to_string((const AiqCanceled&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::CancelReject:
      ret_len = to_string((const CancelReject&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::Executed:
      ret_len = to_string((const Executed&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::ExecutedWithRefPrice:
      ret_len = to_string((const ExecutedWithRefPrice&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::OrderCanceled:
      ret_len = to_string((const OrderCanceled&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::OrderModified:
      ret_len = to_string((const OrderModified&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::OrderReplaced:
      ret_len = to_string((const OrderReplaced&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::SystemEvent:
      ret_len = to_string((const SystemEvent&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::BrokenTrade:
      ret_len = to_string((const BrokenTrade&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::CancelPending:
      ret_len = to_string((const CancelPending&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::PriorityUpdate:
      ret_len = to_string((const PriorityUpdate&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::TradeCorrection:
      ret_len = to_string((const TradeCorrection&)*hdr, buf, sizeof(buf));
      break;

    case InboundMsgType::TradeNow:
      ret_len = to_string((const TradeNow&)*hdr, buf, sizeof(buf));
      break;

    default:
      std::strcpy(buf, "<unknown InboundMsgType>");
      break;
    } // switch

    } // case PacketType::SequencedData
    break;

  default:
    std::strcpy(buf, "<unknown PacketType>");
    break;
  } // switch
  return std::string(buf, ret_len);
}


const std::string
outbound_to_string(const MsgHeader* hdr) {
  char buf[4096];
  size_t ret_len = 4096;

  switch(static_cast<PacketType>(hdr->packet_type)) {
  case PacketType::LoginRequest:
    ret_len = to_string((const LoginRequest&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::ClientHeartbeat:
    ret_len = to_string((const ClientHeartbeat&)*hdr, buf, sizeof(buf));
    break;

  case PacketType::Debug:
    std::strcpy(buf, "Debug...");
    break;

  case PacketType::LogoutRequest:
    std::strcpy(buf, "LogoutRequest...");
    break;

  case PacketType::UnsequencedData: {
    switch(static_cast<OutboundMsgType>(*((const char*)hdr + sizeof(MsgHeader)))) {

    case OutboundMsgType::EnterOrder:
      ret_len = to_string((const EnterOrder&)*hdr, buf, sizeof(buf));
      break;

    case OutboundMsgType::CancelOrder:
      ret_len = to_string((const CancelOrder&)*hdr, buf, sizeof(buf));
      break;

    case OutboundMsgType::ModifyOrder:
      ret_len = to_string((const ModifyOrder&)*hdr, buf, sizeof(buf));
      break;

    case OutboundMsgType::ReplaceOrder:
      ret_len = to_string((const ReplaceOrder&)*hdr, buf, sizeof(buf));
      break;

    default:
      std::strcpy(buf, "<unknown OutboundMsgType>");
      break;
    } // switch
    } // case PacketType::UnsequencedData
    break;

  default:
    std::strcpy(buf, "<unknown PacketType>");
    break;
  } // switch
  return std::string(buf, ret_len);
}
