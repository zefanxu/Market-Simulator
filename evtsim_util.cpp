#include "evtsim_util.h"

string evtsim::Logger::get_datetime_string(){
  time_t t = time(nullptr);
  tm curr_tm = *localtime(&t);
  stringstream ss;
  ss << put_time(&curr_tm, "%Y-%m-%d-%H:%M:%S");
  return ss.str();
}

string evtsim::Logger::get_time_string(){
  time_t t = time(nullptr);
  tm curr_tm = *localtime(&t);
  stringstream ss;
  ss << put_time(&curr_tm, "%H:%M:%S");
  return ss.str();
}

evtsim::Logger::Logger(){
  system("mkdir logs");
  string file_name = "./logs/" + get_datetime_string() + ".txt";
  file.open(file_name, ofstream::out);
  also_print = true;
}

void evtsim::Logger::write(string text){
  string log_text = "[" + get_time_string() + "] " + text + "\n";
  if (also_print)
    cout << log_text;
  file << log_text;
}

void evtsim::Logger::write_warning(string text){
  string log_text = "[" + get_time_string() + "][Warning] "+ text + "\n";
  if (also_print)
    cout << log_text;
  file << log_text;
}

evtsim::Logger::~Logger(){
  file.close();
}

namespace evt {
namespace boe {

namespace { // anonymous

  int to_string(const LoginRequest& m, char* buf, size_t len) {
    return std::snprintf(buf, len,
                  ",session_sub_id=%s,username=%s,password=%s,number_of_param_groups=%hhu,"
                  "order_ack.length=%hu,order_ack.type=0x%02hhx,order_ack.msg_type=0x%02hhx,order_ack.num_bitfields=%hhu,"
                      "order_ack_bitfield1=%hhu,order_ack_bitfield2=%hhu,order_ack_bitfield3=%hhu,order_ack_bitfield4=%hhu,order_ack_bitfield5=%hhu,"
                  "order_modified.length=%hu,order_modified.type=0x%02hhx,order_modified.msg_type=0x%02hhx,order_modified.num_bitfields=%hhu,"
                      "order_modified_bitfield1=%hhu,order_modified_bitfield2=%hhu,order_modified_bitfield3=%hhu,order_modified_bitfield4=%hhu,order_modified_bitfield5=%hhu,"
                  "order_restated.length=%hu,order_restated.type=0x%02hhx,order_restated.msg_type=0x%02hhx,order_restated.num_bitfields=%hhu,"
                      "order_restated_bitfield1=%hhu,order_restated_bitfield2=%hhu,order_restated_bitfield3=%hhu,order_restated_bitfield4=%hhu,order_restated_bitfield5=%hhu,"
                  "order_canceled.length=%hu,order_canceled.type=0x%02hhx,order_canceled.msg_type=0x%02hhx,order_canceled.num_bitfields=%hhu,"
                      "order_canceled_bitfield1=%hhu,order_canceled_bitfield2=%hhu,order_canceled_bitfield3=%hhu,order_canceled_bitfield4=%hhu,order_canceled_bitfield5=%hhu)",
                  // "cancel_rejected.length=%hu,cancel_rejected.type=0x%02hhx,cancel_rejected.msg_type=0x%02hhx,cancel_rejected.num_bitfields=%hhu,\n"
                  //     "cancel_rejected_bitfield1=%hhu,cancel_rejected_bitfield2=%hhu,\n"
                  // "order_execution.length=%hu,order_execution.type=0x%02hhx,order_execution.msg_type=0x%02hhx,order_execution.num_bitfields=%hhu,\n"
                  //     "order_execution_bitfield1=%hhu,order_execution_bitfield2=%hhu,order_execution_bitfield3=%hhu)",
                  std::string(m.session_sub_id, sizeof(m.session_sub_id)).c_str(),
                  std::string(m.username, sizeof(m.username)).c_str(),
                  std::string(m.password, sizeof(m.password)).c_str(), m.number_of_param_groups,
                  m.order_ack.length, m.order_ack.type, m.order_ack.msg_type, m.order_ack.num_bitfields, m.order_ack_bitfield[0], m.order_ack_bitfield[1], m.order_ack_bitfield[2], m.order_ack_bitfield[3], m.order_ack_bitfield[4],
                  m.order_modified.length, m.order_modified.type, m.order_modified.msg_type, m.order_modified.num_bitfields, m.order_modified_bitfield[0], m.order_modified_bitfield[1], m.order_modified_bitfield[2], m.order_modified_bitfield[3], m.order_modified_bitfield[4],
                  m.order_restated.length, m.order_restated.type, m.order_restated.msg_type, m.order_restated.num_bitfields, m.order_restated_bitfield[0], m.order_restated_bitfield[1], m.order_restated_bitfield[2], m.order_restated_bitfield[3], m.order_restated_bitfield[4],
                  m.order_canceled.length, m.order_canceled.type, m.order_canceled.msg_type, m.order_canceled.num_bitfields, m.order_canceled_bitfield[0], m.order_canceled_bitfield[1], m.order_canceled_bitfield[2], m.order_canceled_bitfield[3], m.order_canceled_bitfield[4]);
                  // m.cancel_rejected.length, m.cancel_rejected.type, m.cancel_rejected.msg_type, m.cancel_rejected.num_bitfields, m.cancel_rejected_bitfield[0], m.cancel_rejected_bitfield[1],
                  // m.order_execution.length, m.order_execution.type, m.order_execution.msg_type, m.order_execution.num_bitfields, m.order_execution_bitfield[0], m.order_execution_bitfield[1], m.order_execution_bitfield[2]);
  }

  int to_string(const NewOrder& m, char* buf, size_t len) {
    return std::snprintf(buf, len,
                  ",clordid=%s,side=%c,qty=%u,num_bitfields=%hhu,bitfield1=%hhu,bitfield2=%hhu,bitfield3=%hhu,bitfield4=%hhu,bitfield5=%hhu,bitfield6=%hhu,clearing_firm=%s,clearing_acct=%s,price=%lu,exec_inst=%c,order_type=%c,time_in_force=%c,min_qty=%u,max_floor=%u,symbol=%s,capacity=%c,routing_inst=%s,acct=%s,display_indicator=%c,max_remove_pct=%hhu,discretion_amt=%hu,peg_difference=%ld,attributed_quote=%c,ext_exec_inst=%c)",
                  m.token._str_().c_str(), m.side, m.qty, m.num_bitfields, m.bitfield[0],
                  m.bitfield[1], m.bitfield[2], m.bitfield[3], m.bitfield[4], m.bitfield[5],
                  std::string(m.clearing_firm, sizeof(m.clearing_firm)).c_str(),
                  std::string(m.clearing_acct, sizeof(m.clearing_acct)).c_str(), m.price,
                  (m.exec_inst == 0 ? '0' : m.exec_inst), m.order_type, m.time_in_force, m.min_qty,
                  m.max_floor, std::string(m.symbol, sizeof(m.symbol)).c_str(), m.capacity,
                  std::string(m.routing_inst, sizeof(m.routing_inst)).c_str(),
                  std::string(m.acct, sizeof(m.acct)).c_str(), m.display_indicator,
                  m.max_remove_pct, m.discretion_amt, m.peg_difference, m.attributed_quote,
                  (m.ext_exec_inst == 0 ? '0' : m.ext_exec_inst));
  }

  int to_string(const CancelOrder& m, char* buf, size_t len) {
    return std::snprintf(buf, len, ",orig_clordid=%s,num_bitfields=%hhu,bitfield1=%hhu,clearing_firm=%s)",
                  m.orig_token._str_().c_str(), m.num_bitfields, m.bitfield[0],
                  std::string(m.clearing_firm, sizeof(m.clearing_firm)).c_str());
  }

  int to_string(const ModifyOrder& m, char* buf, size_t len) {
    return std::snprintf(buf, len,
                  ",clordid=%s,orig_clordid=%s,num_bitfields=%hhu,bitfield1=%hhu,bitfield2=%hhu,clearing_firm=%s,qty=%u,price=%lu,order_type=%c,cancel_orig_on_reject=%c,exec_inst=%c,side=%c)",
                  m.token._str_().c_str(), m.orig_token._str_().c_str(), m.num_bitfields,
                  m.bitfield[0], m.bitfield[1],
                  std::string(m.clearing_firm, sizeof(m.clearing_firm)).c_str(), m.qty, m.price,
                  m.order_type, m.cancel_orig_on_reject, (m.exec_inst == 0 ? '0' : m.exec_inst),
                  m.side);
  }

  int to_string(const LoginResponse& m, char* buf, size_t len) {
    int written = std::snprintf(buf, len,
                                ",status=%hhu,text=%s,no_unspecified_unit_replay=%hhu,last_received_seq_num=%u,number_of_units=%hhu,",
                                m.status, std::string(m.text, sizeof(m.text)).c_str(),
                                m.no_unspecified_unit_replay, m.last_received_seq_num,
                                m.number_of_units);
    for(int i = 0; i < static_cast<int>(m.number_of_units); ++i) {
      int n = std::snprintf(buf + written, len - written, "matching_unit=%hhu,seq_num=%u,",
                            m.unit[i].number, m.unit[i].seq_num);
      written += n;
    }
    buf[written-1] = ')';
    buf[written] = '\0';
    return written;
  }

  int to_string(const LogoutResponse& m, char* buf, size_t len) {
    return std::snprintf(buf, len, ",reason=%c,text=%s,last_received_seq_num=%u,number_of_units=%hhu,...",
                  m.reason, std::string(m.text, sizeof(m.text)).c_str(), m.last_received_seq_num,
                  m.number_of_units);
  }

  int to_string(const OrderAck& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,order_id=%lu,reserved=0x%02hhx,num_bitfields=%hhu,"
                  "bitfield1=%hhu,bitfield2=%hhu,bitfield3=%hhu,bitfield4=%hhu,bitfield5=%hhu,"
                  "leaves_qty=%u)",
                  tt, m.token._str_().c_str(), m.order_id, m.reserved, m.num_bitfields,
                  m.bitfield[0], m.bitfield[1], m.bitfield[2], m.bitfield[3], m.bitfield[4],
                  m.leaves_qty);
  }

  int to_string(const OrderModified& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,order_id=%lu,reserved=0x%02hhx,num_bitfields=%hhu,"
                  "bitfield1=%hhu,bitfield2=%hhu,bitfield3=%hhu,bitfield4=%hhu,bitfield5=%hhu,"
                  "price=%lu,qty=%u,leaves_qty=%u)",
                  tt, m.token._str_().c_str(), m.order_id, m.reserved, m.num_bitfields,
                  m.bitfield[0], m.bitfield[1], m.bitfield[2], m.bitfield[3], m.bitfield[4],
                  m.price, m.qty, m.leaves_qty);
  }

  int to_string(const OrderCanceled& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,reason=%c,reserved=0x%02hhx,num_bitfields=%hhu,"
                  "bitfield1=%hhu,bitfield2=%hhu,bitfield3=%hhu,bitfield4=%hhu,bitfield5=%hhu,"
                  "leaves_qty=%u)",
                  tt, m.token._str_().c_str(), m.reason, m.reserved, m.num_bitfields, m.bitfield[0],
                  m.bitfield[1], m.bitfield[2], m.bitfield[3], m.bitfield[4], m.leaves_qty);
  }

  int to_string(const OrderExecution& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,exec_id=%lu,last_shares=%u,last_price=%lu,leaves_qty=%u,base_liquidity_indicator=%c,sub_liquidity_indicator=%c,contra_broker=%s,reserved=0x%hhx,num_bitfields=%hhu)",
                  tt, m.token._str_().c_str(), m.exec_id, m.last_shares, m.last_price, m.leaves_qty,
                  (m.base_liquidity_indicator == '\0' ? '0' : m.base_liquidity_indicator),
                  (m.sub_liquidity_indicator == '\0' ? '0' : m.sub_liquidity_indicator),
                  std::string(m.contra_broker, sizeof(m.contra_broker)).c_str(), m.reserved,
                  m.num_bitfields);
  }

  int to_string(const OrderRejected& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,reason=0x%02hhx,text=%s,reserved=0x%02hhx,num_bitfields=%hhu)",
                  tt, m.token._str_().c_str(), m.reason,
                  std::string(m.text, sizeof(m.text)).c_str(), m.reserved, m.num_bitfields);
  }

  int to_string(const CancelRejected& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,reason=0x%02hhx,text=%s,reserved=0x%02hhx,num_bitfields=%hhu)",
                  tt, m.token._str_().c_str(), m.reason,
                  std::string(m.text, sizeof(m.text)).c_str(), m.reserved, m.num_bitfields);
  }

  int to_string(const ModifyRejected& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,reason=0x%02hhx,text=%s,reserved=0x%02hhx,num_bitfields=%hhu,...",
                  tt, m.token._str_().c_str(), m.reason,
                  std::string(m.text, sizeof(m.text)).c_str(), m.reserved, m.num_bitfields);
  }

  int to_string(const OrderRestated& m, char* buf, size_t len) {
    char tt[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,reason=0x%02hhx,reserved=0x%02hhx,num_bitfields=%hhu,"
                  "bitfield1=%hhu,bitfield2=%hhu,bitfield3=%hhu,bitfield4=%hhu,bitfield5=%hhu,"
                  "qty=%u,leaves_qty=%u)",
                  tt, m.token._str_().c_str(), m.reason, m.reserved, m.num_bitfields,
                  m.bitfield[0], m.bitfield[1], m.bitfield[2], m.bitfield[3], m.bitfield[4],
                  m.qty, m.leaves_qty);
  }

  int to_string(const TradeCancelOrCorrect& m, char* buf, size_t len) {
    char tt[32];
    char ot[32];
    evt::nsec_to_datetime(m.transaction_time, tt, sizeof(tt));
    evt::nsec_to_datetime(m.orig_time, ot, sizeof(ot));
    return std::snprintf(buf, len,
                  ",transaction_time=%s,clordid=%s,order_id=%lu,exec_ref_id=%lu,side=%c,base_liquidity_indicator=%c,clearing_firm=%s,clearing_acct=%s,last_shares=%u,last_price=%lu,corrected_price=%lu,orig_time=%s,reserved=0x%02hhx,num_bitfields=%hhu,...",
                  tt, m.token._str_().c_str(), m.order_id, m.exec_ref_id, m.side,
                  (m.base_liquidity_indicator == '\0' ? '0' : m.base_liquidity_indicator),
                  std::string(m.clearing_firm, sizeof(m.clearing_firm)).c_str(),
                  std::string(m.clearing_acct, sizeof(m.clearing_acct)).c_str(), m.last_shares,
                  m.last_price, m.corrected_price, ot, m.reserved, m.num_bitfields);
  }

} // anonymous



  const std::string
  to_string(const MsgHeader* msg) {
    int ret_len = 0;
    char buf[4096];
    int written = std::snprintf(buf, sizeof(buf),
                                "%s(length=%hu,type=0x%02hhx,matching_unit=%hhu,seq_num=%u",
                                to_string(static_cast<MsgType>(msg->type)), msg->length, msg->type,
                                msg->matching_unit, msg->seq_num);

    const size_t bytes_left = sizeof(buf) - written;

    switch(static_cast<MsgType>(msg->type)) {
    // Member to BOE
    case MsgType::LoginRequest:
      ret_len = to_string((const LoginRequest&)*msg, buf + written, bytes_left);
      break;

    case MsgType::NewOrder:
      ret_len = to_string((const NewOrder&)*msg, buf + written, bytes_left);
      break;

    case MsgType::CancelOrder:
      ret_len = to_string((const CancelOrder&)*msg, buf + written, bytes_left);
      break;

    case MsgType::ModifyOrder:
      ret_len = to_string((const ModifyOrder&)*msg, buf + written, bytes_left);
      break;

    case MsgType::LoginResponse:
      ret_len = to_string((const LoginResponse&)*msg, buf + written, bytes_left);
      break;

    case MsgType::LogoutResponse:
      ret_len = to_string((const LogoutResponse&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderAck:
      ret_len = to_string((const OrderAck&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderModified:
      ret_len = to_string((const OrderModified&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderCanceled:
      ret_len = to_string((const OrderCanceled&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderExecution:
      ret_len = to_string((const OrderExecution&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderRejected:
      ret_len = to_string((const OrderRejected&)*msg, buf + written, bytes_left);
      break;

    case MsgType::CancelRejected:
      ret_len = to_string((const CancelRejected&)*msg, buf + written, bytes_left);
      break;

    case MsgType::ModifyRejected:
      ret_len = to_string((const ModifyRejected&)*msg, buf + written, bytes_left);
      break;

    case MsgType::OrderRestated:
      ret_len = to_string((const OrderRestated&)*msg, buf + written, bytes_left);
      break;

    case MsgType::TradeCancelOrCorrect:
      ret_len = to_string((const TradeCancelOrCorrect&)*msg, buf + written, bytes_left);
      break;

    case MsgType::ClientHeartbeat:
    case MsgType::LogoutRequest:
    case MsgType::ReplayComplete:
    case MsgType::ServerHeartbeat:
    default:
      buf[written] = ')';
      buf[written + 1] = '\0';
      break;
    }
    return string(buf, ret_len + written);
  }

} // boe
} // evt


namespace evt{
namespace ouch{
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
                big_to_native(m.length), m.packet_type, m.msg_type, m.token._str_().c_str(),
                m.side, big_to_native(m.qty), std::string(m.symbol, sizeof(m.symbol)).c_str(),
                big_to_native(m.price), big_to_native(m.time_in_force),
                std::string(m.firm, sizeof(m.firm)).c_str(), m.display, m.capacity,
                  m.intermarket_sweep_eligibility, big_to_native(m.min_qty), m.cross_type,
                  m.customer_type);
  }

int to_string(const CancelOrder& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "CancelOrder(length=%hu,packet_type=%c,msg_type=%c,clordid=%s,qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, m.token._str_().c_str(),
                big_to_native(m.qty));
}
int to_string(const ModifyOrder& m, char* buf, size_t len) {
return std::snprintf(buf, len, "ModifyOrder(length=%hu,packet_type=%c,msg_type=%c,clordid=%s,side=%c,qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, m.token._str_().c_str(),
                m.side, big_to_native(m.qty));
}
int to_string(const ReplaceOrder& m, char* buf, size_t len) {
return std::snprintf(buf, len, "ReplaceOrder(length=%hu,packet_type=%c,msg_type=%c,existing_clordid=%s,clordid=%s,qty=%u,price=%d,time_in_force=%u,display=%c,intermarket_sweep_eligibility=%c,min_qty=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type,
                m.existing_token.val, m.token._str_().c_str(), big_to_native(m.qty),
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
int to_string(const OrderAccepted& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
              "OrderAccepted(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,qty=%u,symbol=%s,price=%d,time_in_force=%u,firm=%s,display=%c,order_reference_number=%lu,capacity=%c,intermarket_sweep_eligibility=%c,min_qty=%u,cross_type=%c,order_state=%c,bbo_weight_indicator=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), m.side, big_to_native(m.qty),
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
                m.token._str_().c_str(), m.reason);
}
int to_string(const ServerHeartbeat& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "ServertHeartbeat(length=%hu,packet_type=%c)", big_to_native(m.length),
                m.packet_type);
}
int to_string(const EndOfSession& m, char* buf, size_t len) {
  return std::snprintf(buf, len, "EndOfSession(length=%hu,packet_type=%c)", big_to_native(m.length),
              m.packet_type);
}
int to_string(const AiqCanceled& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
              "AiqCanceled(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,decrement_qty=%u,reason=%c,qty_prevented_from_trading=%u,execution_price=%u,liquidity_flag=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), big_to_native(m.decrement_qty), m.reason,
                big_to_native(m.qty_prevented_from_trading),
                big_to_native(m.execution_price), m.liquidity_flag);
  }

int to_string(const CancelReject& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "CancelReject(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str());
}
int to_string(const Executed& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "Executed(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_qty=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), big_to_native(m.executed_qty),
                big_to_native(m.execution_price), m.liquidity_flag,
                big_to_native(m.match_number));
  }

int to_string(const ExecutedWithRefPrice& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "ExecutedWithRefPrice(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_qty=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu,reference_price=%u,reference_price_type=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), big_to_native(m.executed_qty),
                big_to_native(m.execution_price), m.liquidity_flag,
                big_to_native(m.match_number), big_to_native(m.reference_price),
                  m.reference_price_type);
  }

int to_string(const OrderCanceled& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderCanceled(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,decrement_qty=%u,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), big_to_native(m.decrement_qty), m.reason);
}
int to_string(const OrderModified& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderModified(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,shares=%u)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), m.side, big_to_native(m.shares));
}
int to_string(const OrderReplaced& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "OrderModified(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,side=%c,qty=%u,symbol=%s,price=%d,time_in_force=%u,firm=%s,display=%c,order_reference_number=%lu,capacity=%c,intermarket_sweep_eligibility=%c,min_qty=%u,cross_type=%c,order_state=%c,orig_clordid=%s,bbo_weight_indicator=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), m.side, big_to_native(m.qty),
                std::string(m.symbol, sizeof(m.symbol)).c_str(), big_to_native(m.price),
                big_to_native(m.time_in_force), std::string(m.firm, sizeof(m.firm)).c_str(),
                  m.display, big_to_native(m.order_reference_number), m.capacity,
                  m.intermarket_sweep_eligibility, big_to_native(m.min_qty), m.cross_type,
                  m.order_state, m.orig_token.val, m.bbo_weight_indicator);
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
                m.token._str_().c_str(), big_to_native(m.match_number), m.reason);
}
int to_string(const CancelPending& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "BrokenTrade(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str());
}
int to_string(const PriorityUpdate& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "PriorityUpdate(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str());
}
int to_string(const TradeCorrection& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "TradeCorrection(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s,executed_shares=%u,execution_price=%d,liquidity_flag=%c,match_number=%lu,reason=%c)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str(), big_to_native(m.executed_shares),
                big_to_native(m.execution_price), m.liquidity_flag, big_to_native(m.match_number),
                m.reason);
  }

int to_string(const TradeNow& m, char* buf, size_t len) {
  return std::snprintf(buf, len,
                "TradeNow(length=%hu,packet_type=%c,msg_type=%c,timestamp=%lu,clordid=%s)",
                big_to_native(m.length), m.packet_type, m.msg_type, big_to_native(m.timestamp),
                m.token._str_().c_str());
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
}}
