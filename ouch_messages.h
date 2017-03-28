#pragma once
#include <cstdint>
#include <string>
#include <boost/enum.hpp>
#include <boost/endian/conversion.hpp>

using boost::endian::big_to_native;
using boost::endian::native_to_big;

#define MAX_SHARES 999999

namespace evt {
namespace ouch {

  // SoupBinTCP types
  enum class PacketType : char {
    Debug = '+',

    // Client to ouch
    UnsequencedData = 'U',
    ClientHeartbeat = 'R',
    LoginRequest = 'L',
    LogoutRequest = 'O',

    // Ouch to client
    SequencedData = 'S',
    EndOfSession = 'Z',
    LoginAccepted = 'A',
    LoginRejected = 'J',
    ServerHeartbeat = 'H',
  };
  inline const char*
  to_string(PacketType e) {
    switch(e) {
    case PacketType::ClientHeartbeat: return "ClientHeartbeat";
    case PacketType::Debug: return "Debug";
    case PacketType::EndOfSession: return "EndOfSession";
    case PacketType::LoginAccepted: return "LoginAccepted";
    case PacketType::LoginRejected: return "LoginRejected";
    case PacketType::LoginRequest: return "LoginRequest";
    case PacketType::LogoutRequest: return "LogoutRequest";
    case PacketType::ServerHeartbeat: return "ServerHeartbeat";
    case PacketType::SequencedData: return "SequencedData"; // inbound
    case PacketType::UnsequencedData: return "UnsequencedData"; // outbound
    }
    return "<unknown PacketType>";
  }

  // Nasdaq outbound OUCH types (client to ouch)
  enum class OutboundMsgType : char {
    CancelOrder = 'X',
    EnterOrder = 'O',
    ModifyOrder = 'M',
    ReplaceOrder = 'U',
  };
  inline const char*
  to_string(OutboundMsgType e) {
    switch(e) {
    case OutboundMsgType::CancelOrder: return "CancelOrder";
    case OutboundMsgType::EnterOrder: return "EnterOrder";
    case OutboundMsgType::ModifyOrder: return "ModifyOrder";
    case OutboundMsgType::ReplaceOrder: return "ReplaceOrder";
    }
    return "<unknown OutboundMsgType>";
  }

  // Nasdaq inbound OUCH types (ouch to client)
  enum class InboundMsgType : char {
    AiqCanceled = 'D',
    BrokenTrade = 'B',
    CancelPending = 'P',
    CancelReject = 'I',
    Executed = 'E',
    ExecutedWithRefPrice = 'G',
    OrderAccepted = 'A',
    OrderCanceled = 'C',
    OrderModified = 'M',
    OrderRejected = 'J',
    OrderReplaced = 'U',
    PriorityUpdate = 'T',
    SystemEvent = 'S',
    TradeCorrection = 'F',
    TradeNow = 'N',
  };
  inline const char*
  to_string(InboundMsgType e) {
    switch(e) {
    case InboundMsgType::AiqCanceled: return "AiqCanceled";
    case InboundMsgType::BrokenTrade: return "BrokenTrade";
    case InboundMsgType::CancelPending: return "CancelPending";
    case InboundMsgType::CancelReject: return "CancelReject";
    case InboundMsgType::Executed: return "Executed";
    case InboundMsgType::ExecutedWithRefPrice: return "ExecutedWithRefPrice";
    case InboundMsgType::OrderAccepted: return "OrderAccepted";
    case InboundMsgType::OrderCanceled: return "OrderCanceled";
    case InboundMsgType::OrderModified: return "OrderModified";
    case InboundMsgType::OrderRejected: return "OrderRejected";
    case InboundMsgType::OrderReplaced: return "OrderReplaced";
    case InboundMsgType::PriorityUpdate: return "PriorityUpdate";
    case InboundMsgType::SystemEvent: return "SystemEvent";
    case InboundMsgType::TradeCorrection: return "TradeCorrection";
    case InboundMsgType::TradeNow: return "TradeNow";
    }
    return "<unknown InboundMsgType>";
  }

  enum TimeInForce : int {
    Ioc = 0,        // immediate or cancel
    Market = 99998, // until market close
    System = 99999, // until end of trading day
  };
  BOOST_ENUM_VALUES(EventCode, char,
    (StartOfDay)('S')
    (EndOfDay)('E')
  );

  BOOST_ENUM(OrderType,
    (Limit)
    (Market)
    (Algo)
    (PegToMid)
    (PegToBid)
    (PegToAsk)
    (OnOpen)
    (OnClose)
    (ISO)
  );

  BOOST_ENUM(InventoryFlag,
    (Init)
    (Cover)
  );

  BOOST_ENUM(OrderDisplay,
    (Normal)
    (Hidden)
  );

  BOOST_ENUM(OrderCapacity,
    (Agent)
    (Principal)
  );

  BOOST_ENUM_VALUES(LiquidityCode, char,
    (Unknown)('U')
    (AddLit)('A')
    (RemoveLit)('R')
    (AddDark)('a')
    (RemoveDark)('r')
    (AddMidpoint)('m')
    (TakeMidpoint)('M')
  );

  enum Side : char {
    Buy = 'B',
    Sell = 'S',
    SellShort = 'T',
    SellShortExempt = 'E'
  };

  BOOST_ENUM_VALUES(Display, char,
    (AttributablePriceToDisplay)('A')
    (AnonymousPriceToComply)('Y')
    (NonDisplay)('N')
    (PostOnly)('P')
    (ImbalanceOnly)('I')
    (MidpointPeg)('M')
    (MidpointPegPostOnly)('W')
    (PostOnlyAndAttributable)('L')
    (RetailOrderType1)('O')
    (RetailOrderType2)('T')
    (RetailPriceImprovement)('Q')
  );

  BOOST_ENUM_VALUES(Capacity, char,
    (Agency)('A')
    (Principal)('P')
    (Riskless)('R')
    (Other)('O')
  );

  BOOST_ENUM_VALUES(CrossType, char,
    (NoCrossContinuousMarket)('N')
    (OpeningCross)('O')
    (ClosingCross)('C')
    (HaltOrIpoCross)('H')
    (SupplementalOrder)('S')
    (Retail)('R')
  );

  enum OrderState : char {
    Live = 'L',
    Dead = 'D',
  };


  BOOST_ENUM_VALUES(RejectedReason, char,
    (TestMode)('T')
    (Halted)('H')
    (ExceededConfiguredSafety)('Z')
    (InvalidStock)('S')
    (InvalidDisplayType)('D')
    (OuchClosed)('C')
    (RequestedFirmNotAuthorized)('L')
    (ClearingTypeOutsideOfPermittedTimes)('L')
    (OrderNotAllowedInTypeOfCross)('R')
    (InvalidPrice)('X')
    (InvalidMinQty)('N')
    (Other)('O')
    (InvalidMidpointPostOnlyPrice)('W')

    // PRM Rejected Order Reasons
    (RejectAllEnabled)('a')
    (EasyToBorrowReject)('b')
    (RestrictedSymbolList)('c')
    (IsoOrderRestriction)('d')
    (OddLotRestriction)('e')
    (MidpointRestriction)('f')
    (PreMarketRestriction)('g')
    (PostMarketRestriction)('h')
    (ShortSaleRestriction)('i')
    (OnOpenRestriction)('j')
    (OnCloseRestriction)('k')
    (TwoSidedQuoteReject)('l')
    (ExceededSharesLimit)('m')
    (ExceededDollarValue)('n')
  );

  struct Token {
    std::string _str_() const{
      return std::string(val, 14);
    }
    char val[14];
  } __attribute__((packed));

  #define OUCH_MSG_HEADER\
    uint16_t length;\
    char packet_type;

  struct MsgHeader{
    OUCH_MSG_HEADER
  }__attribute__((packed));

  struct Ouch_MsgHeader{
    OUCH_MSG_HEADER
    char msg_type;
  }__attribute__((packed));

  // Session-level (SoupbinTCP) messages
  // Inbound (ouch to client)
  // ******************************************************************

  struct LoginAccepted{
    LoginAccepted(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginAccepted)-2))), packet_type(static_cast<char>(PacketType::LoginAccepted)){}
    OUCH_MSG_HEADER
    char session[10]; // left-padded with spaces
    char seq_num[20]; // left-padded with spaces
  } __attribute__((packed));


  struct LoginRejected{
    LoginRejected(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginRejected)-2))), packet_type(static_cast<char>(PacketType::LoginRejected)){}
    OUCH_MSG_HEADER
    char reason;
  } __attribute__((packed));

  struct ServerHeartbeat{
    ServerHeartbeat(): length(native_to_big(static_cast<uint16_t>(sizeof(ServerHeartbeat)-2))), packet_type(static_cast<char>(PacketType::ServerHeartbeat)){}
    OUCH_MSG_HEADER
  } __attribute__((packed));

  struct EndOfSession{
    EndOfSession(): length(native_to_big(static_cast<uint16_t>(sizeof(EndOfSession)-2))), packet_type(static_cast<char>(PacketType::EndOfSession)){}
    OUCH_MSG_HEADER
  } __attribute__((packed));


  // Outbound (client to ouch)
  // ******************************************************************

  struct LoginRequest{
    LoginRequest(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginRequest)-2))), packet_type(static_cast<char>(PacketType::LoginRequest)){}
    OUCH_MSG_HEADER
    char username[6]; // right-padded with spaces
    char password[10]; // right-padded with spaces
    char requested_session[10]; // left-padded with spaces
    char requested_seq_num[20]; // left-padded with spaces
  } __attribute__((packed));


  struct ClientHeartbeat{
    ClientHeartbeat(): length(native_to_big(static_cast<uint16_t>(sizeof(ClientHeartbeat)-2))), packet_type(static_cast<char>(PacketType::ClientHeartbeat)){}
    OUCH_MSG_HEADER
  } __attribute__((packed));


  struct LogoutRequest{
    LogoutRequest(): length(native_to_big(static_cast<uint16_t>(sizeof(LogoutRequest)-2))), packet_type(static_cast<char>(PacketType::LogoutRequest)){}
    OUCH_MSG_HEADER
  } __attribute__((packed));


  // Application-level (Ouch) messages
  // Inbound (client to ouch)
  // ******************************************************************

  struct EnterOrder{
    EnterOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(EnterOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::EnterOrder)){}
    void from_network(){
      qty = big_to_native(qty);
      price = big_to_native(price);
      time_in_force = big_to_native(time_in_force);
      min_qty = big_to_native(min_qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Token token;
    char side; ///< \see ouch::Side
    uint32_t qty; // 0 <= qty <= 1,000,000
    char symbol[8];
    int32_t price;
    uint32_t time_in_force; ///< \see ouch::TimeInForce
    char firm[4]; // all caps
    char display; ///< \see ouch::Display
    char capacity; ///< \see ouch::Capacity
    char intermarket_sweep_eligibility; ///< \see ouch::IntermarketSweepEligibility
    uint32_t min_qty;
    char cross_type; ///< \see ouch::CrossType
    char customer_type;
  } __attribute__((packed));


  struct ReplaceOrder{
    ReplaceOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(ReplaceOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::ReplaceOrder)){}
    void from_network(){
      qty = big_to_native(qty);
      price = big_to_native(price);
      time_in_force = big_to_native(time_in_force);
      min_qty = big_to_native(min_qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Token existing_token;
    Token token;
    uint32_t qty; // 0 <= qty <= 1,000,000
    int32_t price;
    uint32_t time_in_force; ///< \see ouch::TimeInForce
    char display; ///< \see ouch::Display
    char intermarket_sweep_eligibility; ///< \see ouch::IntermarketSweepEligibility
    uint32_t min_qty;
  } __attribute__((packed));


  struct CancelOrder{
    CancelOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(CancelOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::CancelOrder)){}
    void from_network(){
      qty = big_to_native(qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Token token;
    uint32_t qty; // 0 <= qty <= 1,000,000
  } __attribute__((packed));


  struct ModifyOrder{
    ModifyOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(ModifyOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::ModifyOrder)){}
    void from_network(){
      qty = big_to_native(qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Token token;
    char side; ///< \see ouch::Side
    uint32_t qty; // 0 <= qty <= 1,000,000
  } __attribute__((packed));


  // Inbound (ouch to client)
  // ******************************************************************

  struct SystemEvent{
    SystemEvent(): length(native_to_big(static_cast<uint16_t>(sizeof(SystemEvent)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::SystemEvent)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp; // nsecs past midnight
    char event_code; ///< \see ouch::EventCode
  } __attribute__((packed));


  struct OrderAccepted{
    OrderAccepted(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderAccepted)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderAccepted)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      price = native_to_big(price);
      qty = native_to_big(qty);
      time_in_force = native_to_big(time_in_force);
      order_reference_number = native_to_big(order_reference_number);
      min_qty = native_to_big(min_qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp; // nsecs past midnight
    Token token;
    char side; ///< \see ouch::Side
    uint32_t qty; // 0 <= qty <= 1,000,000
    char symbol[8];
    int32_t price;
    uint32_t time_in_force; ///< \see ouch::TimeInForce
    char firm[4]; // all caps
    char display; ///< \see ouch::Display
    uint64_t order_reference_number;
    char capacity; ///< \see ouch::Capacity
    char intermarket_sweep_eligibility; ///< \see ouch::IntermarketSweepEligibility
    uint32_t min_qty;
    char cross_type; ///< \see ouch::CrossType
    char order_state; ///< \see ouch::OrderState
    char bbo_weight_indicator;
  } __attribute__((packed));


  struct OrderCanceled{
    OrderCanceled(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderCanceled)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderCanceled)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      decrement_qty = native_to_big(decrement_qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint32_t decrement_qty;
    char reason;
  } __attribute__((packed));


  struct OrderModified{
    OrderModified(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderModified)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderModified)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      shares = native_to_big(shares);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    char side; ///< \see ouch::Side
    uint32_t shares; // 0 <= shares <= 1,000,000
  } __attribute__((packed));


  struct OrderRejected{
    OrderRejected(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderRejected)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderRejected)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    char reason; ///< \see ouch::RejectedReason
  } __attribute__((packed));


  struct OrderReplaced{
    OrderReplaced(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderReplaced)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderReplaced)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      price = native_to_big(price);
      qty = native_to_big(qty);
      time_in_force = native_to_big(time_in_force);
      min_qty = native_to_big(min_qty);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token; ///< called Replacement Token in specs
    char side; ///< \see ouch::Side
    uint32_t qty; // 0 <= qty <= 1,000,000
    char symbol[8];
    int32_t price;
    uint32_t time_in_force; ///< \see ouch::TimeInForce
    char firm[4]; // all caps
    char display; ///< \see ouch::Display
    uint64_t order_reference_number;
    char capacity; ///< \see ouch::Capacity
    char intermarket_sweep_eligibility; ///< \see ouch::IntermarketSweepEligibility
    uint32_t min_qty;
    char cross_type; ///< \see ouch::CrossType
    char order_state; ///< \see ouch::OrderState
    Token orig_token; ///< called Previous Token in specs
    char bbo_weight_indicator;
  } __attribute__((packed));


  struct Executed{
    Executed(): length(native_to_big(static_cast<uint16_t>(sizeof(Executed)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::Executed)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      execution_price = native_to_big(execution_price);
      executed_qty = native_to_big(executed_qty);
      match_number = native_to_big(match_number);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint32_t executed_qty;
    int32_t execution_price;
    char liquidity_flag; ///< \see ouch::LiquidityFlag
    uint64_t match_number;
  } __attribute__((packed));


  /// Same as Executed, but with an additional ref price
  struct ExecutedWithRefPrice{
    ExecutedWithRefPrice(): length(native_to_big(static_cast<uint16_t>(sizeof(ExecutedWithRefPrice)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::ExecutedWithRefPrice)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      execution_price = native_to_big(execution_price);
      executed_qty = native_to_big(executed_qty);
      reference_price = native_to_big(reference_price);
      match_number = native_to_big(match_number);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint32_t executed_qty;
    int32_t execution_price;
    char liquidity_flag; ///< \see ouch::LiquidityFlag
    uint64_t match_number;
    uint32_t reference_price;
    char reference_price_type;
  } __attribute__((packed));


  struct BrokenTrade{
    BrokenTrade(): length(native_to_big(static_cast<uint16_t>(sizeof(BrokenTrade)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::BrokenTrade)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      match_number = native_to_big(match_number);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint64_t match_number;
    char reason; ///< \see ouch::BrokenTradeReason
  } __attribute__((packed));


  struct CancelPending{
    CancelPending(): length(native_to_big(static_cast<uint16_t>(sizeof(CancelPending)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::CancelPending)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
  } __attribute__((packed));


  struct CancelReject{
    CancelReject(): length(native_to_big(static_cast<uint16_t>(sizeof(CancelReject)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::CancelReject)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
  } __attribute__((packed));


  struct AiqCanceled{
    AiqCanceled(): length(native_to_big(static_cast<uint16_t>(sizeof(AiqCanceled)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::AiqCanceled)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      decrement_qty = native_to_big(decrement_qty);
      qty_prevented_from_trading = native_to_big(qty_prevented_from_trading);
      execution_price = native_to_big(execution_price);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint32_t decrement_qty;
    char reason;
    uint32_t qty_prevented_from_trading;
    uint32_t execution_price;
    char liquidity_flag;
  } __attribute__((packed));


  struct PriorityUpdate{
    PriorityUpdate(): length(native_to_big(static_cast<uint16_t>(sizeof(PriorityUpdate)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::PriorityUpdate)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      order_reference_number = native_to_big(order_reference_number);
      price = native_to_big(price);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    int32_t price;
    char display;
    uint64_t order_reference_number;
  } __attribute__((packed));


  struct TradeCorrection{
    TradeCorrection(): length(native_to_big(static_cast<uint16_t>(sizeof(TradeCorrection)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::TradeCorrection)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
      executed_shares = native_to_big(executed_shares);
      execution_price = native_to_big(execution_price);
      match_number = native_to_big(match_number);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
    uint32_t executed_shares;
    int32_t execution_price;
    char liquidity_flag;
    uint64_t match_number;
    char reason;
  } __attribute__((packed));


  struct TradeNow{
    TradeNow(): length(native_to_big(static_cast<uint16_t>(sizeof(TradeNow)-2))),
        packet_type(static_cast<char>(PacketType::SequencedData)),
        msg_type(static_cast<char>(InboundMsgType::TradeNow)){}
    void to_network(){
      timestamp = native_to_big(timestamp);
    }
    OUCH_MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Token token;
  } __attribute__((packed));
} // namespace ouch
} // namespace evt
