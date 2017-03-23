#pragma once

#include <cstdint>
#include <string>

// All data is put on the wire in little-endian byte order.

namespace evt {
namespace boe {

  struct Token {
    std::string _str_(){
      return std::string(val, 20);
    }
    char val[20];
  } __attribute__((packed));

  #define BOE_MSG_HEADER\
  unsigned char start_of_msg[2] = {0xBA, 0xBA};\
  uint16_t length;\
  uint8_t type;\
  uint8_t matching_unit;\
  uint32_t seq_num;

  struct MsgHeader {
    BOE_MSG_HEADER
  } __attribute__((packed));


  struct UnitSequencesParamGroup {
    uint16_t length;
    uint8_t type;
    uint8_t no_unspecified_unit_replay;
    uint8_t number_of_units;
  } __attribute__((packed));


  struct ReturnBitfieldParamGroup {
    uint16_t length;
    uint8_t type;
    uint8_t msg_type;
    uint8_t num_bitfields;
  } __attribute__((packed));


  // Session Msgs - Member to BOE
  /**********************************************************************/

  struct LoginRequest {
    BOE_MSG_HEADER
    char session_sub_id[4];
    char username[4];
    char password[10];
    uint8_t number_of_param_groups;
    UnitSequencesParamGroup replay;
    ReturnBitfieldParamGroup order_ack;
    uint8_t order_ack_bitfield[5];
    ReturnBitfieldParamGroup order_modified;
    uint8_t order_modified_bitfield[5];
    ReturnBitfieldParamGroup order_restated;
    uint8_t order_restated_bitfield[5];
    ReturnBitfieldParamGroup order_canceled;
    uint8_t order_canceled_bitfield[5];
    // ReturnBitfieldParamGroup cancel_rejected;
    // uint8_t cancel_rejected_bitfield[2];
    // ReturnBitfieldParamGroup order_execution;
    // uint8_t order_execution_bitfield[3];
  } __attribute__((packed));


  // NOTE: Both LogoutRequest and ClientHeartbeat are "empty" structs
  // which don't add any fields after the header. There's probably a
  // clever typedef trick that allows us to treat them as separate types
  // in the header but one type in the cpp, but I didn't spend too much
  // time trying to figure it out.
  struct LogoutRequest {
    BOE_MSG_HEADER
  } __attribute__((packed));


  struct ClientHeartbeat {
    BOE_MSG_HEADER
  } __attribute__((packed));


  // Session Msgs - BOE to Member
  /**********************************************************************/
  struct UnitSequence {
    uint8_t number;
    uint32_t seq_num;
  } __attribute__((packed));


  struct LoginResponse {
    BOE_MSG_HEADER
    uint8_t status;
    char text[60];
    uint8_t no_unspecified_unit_replay;
    uint32_t last_received_seq_num;
    uint8_t number_of_units;
    UnitSequence unit[0];
  } __attribute__((packed));


  struct LogoutResponse {
    BOE_MSG_HEADER
    char reason;
    char text[60];
    uint32_t last_received_seq_num;
    uint8_t number_of_units;
    UnitSequence unit[32];
  } __attribute__((packed));


  // NOTE: See note above regarding typedefs for LogoutRequest and
  // ClientHeartbeat.
  struct ServerHeartbeat {
    BOE_MSG_HEADER
  } __attribute__((packed));


  struct ReplayComplete{
    BOE_MSG_HEADER
  } __attribute__((packed));


  // Application Msgs - Member to BOE
  /**********************************************************************/

  struct NewOrder {
    BOE_MSG_HEADER
    Token token;
    char side;
    uint32_t qty;
    uint8_t num_bitfields;
    uint8_t bitfield[6];
    char clearing_firm[4];
    char clearing_acct[4];
    uint64_t price; // 4 implied decimal places
    char exec_inst;
    char order_type;
    char time_in_force;
    uint32_t min_qty;
    uint32_t max_floor;
    char symbol[8];
    char capacity;
    char routing_inst[4];
    char acct[16];
    char display_indicator;
    uint8_t max_remove_pct;
    uint16_t discretion_amt;
    int64_t peg_difference;
    char attributed_quote;
    char ext_exec_inst;
  } __attribute__((packed));


  struct CancelOrder {
    BOE_MSG_HEADER
    Token orig_clordid;
    uint8_t num_bitfields;
    uint8_t bitfield[1];
    char clearing_firm[4];
  } __attribute__((packed));


  struct ModifyOrder {
    BOE_MSG_HEADER
    Token token;
    Token orig_clordid;
    uint8_t num_bitfields;
    uint8_t bitfield[2];
    char clearing_firm[4];
    uint32_t qty;
    uint64_t price;
    char order_type;
    char cancel_orig_on_reject;
    char exec_inst;
    char side;
  } __attribute__((packed));


  // Application Msgs - BOE to Member
  /**********************************************************************/

  struct OrderAck {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint64_t order_id;
    uint8_t reserved;
    uint8_t num_bitfields;
    uint8_t bitfield[5];
    uint32_t leaves_qty;
  } __attribute__((packed));


  struct OrderRejected {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint8_t reason;
    char text[60];
    uint8_t reserved;
    uint8_t num_bitfields;
  } __attribute__((packed));


  struct OrderModified {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint64_t order_id;
    uint8_t reserved;
    uint8_t num_bitfields;
    uint8_t bitfield[5];
    uint64_t price; // 4 implied decimal places
    uint32_t qty;
    uint32_t leaves_qty;
  } __attribute__((packed));


  struct OrderRestated {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint64_t order_id;
    uint8_t reason;
    uint8_t reserved;
    uint8_t num_bitfields;
    uint8_t bitfield[5];
    uint32_t qty;
    uint32_t leaves_qty;
  } __attribute__((packed));


  struct ModifyRejected {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint8_t reason;
    char text[60];
    uint8_t reserved;
    uint8_t num_bitfields;
  } __attribute__((packed));


  struct OrderCanceled {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    char reason;
    uint8_t reserved;
    uint8_t num_bitfields;
    uint8_t bitfield[5];
    uint32_t leaves_qty;
  } __attribute__((packed));


  struct CancelRejected {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint8_t reason;
    char text[60];
    uint8_t reserved;
    uint8_t num_bitfields;
  } __attribute__((packed));


  struct OrderExecution {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint64_t exec_id;
    uint32_t last_shares;
    uint64_t last_price;
    uint32_t leaves_qty;
    char base_liquidity_indicator;
    char sub_liquidity_indicator;
    char contra_broker[4];
    uint8_t reserved;
    uint8_t num_bitfields;
  } __attribute__((packed));


  struct TradeCancelOrCorrect {
    BOE_MSG_HEADER
    uint64_t transaction_time;
    Token token;
    uint64_t order_id;
    uint64_t exec_ref_id;
    char side;
    char base_liquidity_indicator;
    char clearing_firm[4];
    char clearing_acct[4];
    uint32_t last_shares;
    uint64_t last_price;
    uint64_t corrected_price;
    uint64_t orig_time;
    uint8_t reserved;
    uint8_t num_bitfields;
    uint8_t bitfield[2];
    char symbol[8];
  } __attribute__((packed));


enum class LoginResponseStatus : char {
    Accepted = 'A',
    NotAuthorized = 'N',
    SessionDisabled = 'D',
    SessionInUse = 'B',
    InvalidSession = 'S',
    SequenceAhead = 'Q',
    InvalidUnitGiven = 'I',
    InvalidReturnBitfield = 'F',
    InvalidRequestStructure = 'M',
  };
  inline const char*
  to_string(LoginResponseStatus e) {
    switch(e) {
    case LoginResponseStatus::Accepted: return "Login accepted";
    case LoginResponseStatus::NotAuthorized: return "Not authorized (invalid username/password)";
    case LoginResponseStatus::SessionDisabled: return "Session is disable";
    case LoginResponseStatus::SessionInUse: return "Session in use";
    case LoginResponseStatus::InvalidSession: return "Invalid session";
    case LoginResponseStatus::SequenceAhead: return "Sequence ahead in login message";
    case LoginResponseStatus::InvalidUnitGiven: return "Invalid unit given in login message";
    case LoginResponseStatus::InvalidReturnBitfield: return "Invalid return bitfield in login message";
    case LoginResponseStatus::InvalidRequestStructure: return "Invalid login request message structure";
    default: assert(false);
    }
  }


  enum class MsgType : uint8_t {
    // Member to boe
    CancelOrder = 0x39,
    ClientHeartbeat = 0x03,
    LoginRequest = 0x37,
    LogoutRequest = 0x02,
    ModifyOrder = 0x3a,
    NewOrder = 0x38,

    // boe to member
    CancelRejected = 0x2b,
    LoginResponse = 0x24,
    LogoutResponse = 0x08,
    ModifyRejected = 0x29,
    OrderAck = 0x25,
    OrderCanceled = 0x2a,
    OrderExecution = 0x2c,
    OrderModified = 0x27,
    OrderRejected = 0x26,
    OrderRestated = 0x28,
    ReplayComplete = 0x13,
    ServerHeartbeat = 0x09,
    TradeCancelOrCorrect = 0x2d,
  };

  inline const char*
  to_string(MsgType e) {
    switch(e) {
    case MsgType::CancelOrder: return "CancelOrder";
    case MsgType::CancelRejected: return "CancelRejected";
    case MsgType::ClientHeartbeat: return "ClientHeartbeat";
    case MsgType::LoginRequest: return "LoginRequest";
    case MsgType::LoginResponse: return "LoginResponse";
    case MsgType::LogoutRequest: return "LogoutRequest";
    case MsgType::LogoutResponse: return "LogoutResponse";
    case MsgType::ModifyOrder: return "ModifyOrder";
    case MsgType::ModifyRejected: return "ModifyRejected";
    case MsgType::NewOrder: return "NewOrder";
    case MsgType::OrderAck: return "OrderAck";
    case MsgType::OrderCanceled: return "OrderCanceled";
    case MsgType::OrderExecution: return "OrderExecution";
    case MsgType::OrderModified: return "OrderModified";
    case MsgType::OrderRejected: return "OrderRejected";
    case MsgType::OrderRestated: return "OrderRestated";
    case MsgType::ReplayComplete: return "ReplayComplete";
    case MsgType::ServerHeartbeat: return "ServerHeartbeat";
    case MsgType::TradeCancelOrCorrect: return "TradeCancelOrCorrect";
    default: return "<unknown>";
    }
  }


  enum class ParamGroupType : uint8_t {
    ReturnBitfields = 0x81,
    UnitSequences = 0x80,
  };


  enum class Reason : char {
    Administrative = 'A',
    CapacityUndefined = 'C',
    DuplicateIdentifier = 'D',
    SizeReductionDueToSwpRestatement = 'E',
    Halted = 'H',
    IncorrectDataCenter = 'I',
    TooLateToCancel = 'J',
    OrderRateThresholdExceeded = 'K',
    OrderWouldLockOrCrossNbbo = 'L',
    LiquidityAvailableExceedsOrderSize = 'M',
    RanOutOfLiquidityToExecuteAgainst = 'N',
    ClordidDoesntMatchAKnownOrder = 'O',
    CantModifyAnOrderThatIsPendingFill = 'P',
    WaitingForFirstTrade = 'Q',
    RoutingUnavailable = 'R',
    ShortSalePriceViolation = 'S',
    FillWouldTradeThroughTheNbbo = 'T',
    UserRequested = 'U',
    WouldWash = 'V',
    AddLiquidityOnlyOrderWouldRemove = 'W',
    OrderExpired = 'X',
    SymbolNotSupported = 'Y',
    UnforeseenReason = 'Z',
    MarketAccessRiskLimitExceeded = 'm',
    MaxOpenOrdersCountExceeded = 'o',
    ReserveReload = 'r',
    RiskManagementSymbolLevel = 's',
    LimitUpDown = 'u',
    WouldRemoveOnUnslide = 'w',
    CrossedMarket = 'x',
    OrderReceivedByBatsDuringReplay = 'y',
    ProtocolViolation = '!',
  };

  inline const char*
  to_string(Reason e) {
    switch(e) {
    case Reason::Administrative: return "Administrative";
    case Reason::CapacityUndefined: return "Capacity undefined";
    case Reason::DuplicateIdentifier: return "Duplicate identifier";
    case Reason::SizeReductionDueToSwpRestatement: return "Size reduction due to swp restatement";
    case Reason::Halted: return "Halted";
    case Reason::IncorrectDataCenter: return "Incorrect data center";
    case Reason::TooLateToCancel: return "Too late to cancel";
    case Reason::OrderRateThresholdExceeded: return "Order rate threshold exceeded";
    case Reason::OrderWouldLockOrCrossNbbo: return "Order would lock or cross nbbo";
    case Reason::LiquidityAvailableExceedsOrderSize: return "Liquidity available exceeds order size";
    case Reason::RanOutOfLiquidityToExecuteAgainst: return "Ran out of liquidity to execute against";
    case Reason::ClordidDoesntMatchAKnownOrder: return "Clordid doesn't match a known order";
    case Reason::CantModifyAnOrderThatIsPendingFill: return "Can't modify an order that is pending fill";
    case Reason::WaitingForFirstTrade: return "Waiting for first trade";
    case Reason::RoutingUnavailable: return "Routing unavailable";
    case Reason::ShortSalePriceViolation: return "Short sale price violation";
    case Reason::FillWouldTradeThroughTheNbbo: return "Fill would trade through the nbbo";
    case Reason::UserRequested: return "User requested";
    case Reason::WouldWash: return "Would wash";
    case Reason::AddLiquidityOnlyOrderWouldRemove: return "Add liquidity only order would remove";
    case Reason::OrderExpired: return "Order expired";
    case Reason::SymbolNotSupported: return "Symbol not supported";
    case Reason::UnforeseenReason: return "Unforeseen reason";
    case Reason::MarketAccessRiskLimitExceeded: return "Market access risk limit exceeded";
    case Reason::MaxOpenOrdersCountExceeded: return "Max open orders count exceeded";
    case Reason::ReserveReload: return "Reserve reload";
    case Reason::RiskManagementSymbolLevel: return "Risk management symbol level";
    case Reason::LimitUpDown: return "Limit up down";
    case Reason::WouldRemoveOnUnslide: return "Would remove on unslide";
    case Reason::CrossedMarket: return "Crossed market";
    case Reason::OrderReceivedByBatsDuringReplay: return "Order received by bats during replay";
    case Reason::ProtocolViolation: return "Protocol Violation";
    default: return "";
    }
  }


  enum class Side : char {
    Buy = '1',
    Sell = '2',
    SellShort = '5',
    SellShortExempt = '6',
  };

  enum class TimeInForce : char {
    Day = '0',
    GTC = '1',
    AtTheOpen = '2',
    IOC = '3',
    FOK = '4',
    GTX = '5', // expires at end of extended day
    GTD = '6',
    AtTheClose = '7',
    Pre = 'E',
    RHO = 'R', // regular hours only
    PTD = 'T', // pre-market trading session
    PTX = 'X', // pre-market until end of post-market
  };

  enum DisplayIndicator : char {
    Normal = 'V', // specified by port config
    Hidden = 'r',
  };

  enum class Capacity : char {
    Agency = 'A',
    Principal = 'P',
    RisklessPrincipal = 'R',
  };


  enum class OrderType : char {
    Market = '1',
    Limit = '2',
    Stop = '3',
    StopLimit = '4',
    Pegged = 'P',
  };

} // boe
} // evt
