#pragma once
#include <cstdint>
#include <string>
#include <boost/enum.hpp>
#include <boost/endian/conversion.hpp>
#include "../evts/evts_types.h"
#include "../evts/order.h"

using boost::endian::big_to_native;
using boost::endian::native_to_big;

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
  inline bool
  convert_from_evt_tif(const evt::OrderTIF& in, ouch::TimeInForce& out) {
    switch(in.index()) {
    case evt::OrderTIF::Day: out = ouch::TimeInForce::Market; break;
    case evt::OrderTIF::IOC: out = ouch::TimeInForce::Ioc; break;
    case evt::OrderTIF::FillOrKill: out = ouch::TimeInForce::Ioc; break;
    default: return false;
    }
    return true;
  }
  inline bool
  convert_to_evt_tif(ouch::TimeInForce in, evt::OrderTIF& out) {
    switch(in) {
    case ouch::TimeInForce::Market: out = evt::OrderTIF::Day; break;
    case ouch::TimeInForce::Ioc: out = evt::OrderTIF::IOC; break;
    default: return false;
    }
    return true;
  }


  BOOST_ENUM_VALUES(Side, char,
    (Buy)('B')
    (Sell)('S')
    (SellShort)('T')
    (SellShortExempt)('E')
  );
  inline bool
  convert_from_evt_side(const evt::OrderSide& in, ouch::Side& out) {
    switch(in.index()) {
    case OrderSide::Buy:    out = Side::Buy; break;
    case OrderSide::Sell:   out = Side::Sell; break;
    case OrderSide::Short:  out = Side::SellShort; break;
    default: return false;
    }
    return true;
  }


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
  inline bool
  convert_from_evt_display(const evt::OrderDisplay& in, ouch::Display& out) {
    switch(in.index()) {
    case OrderDisplay::Normal:  out = Display::AttributablePriceToDisplay; break;
    case OrderDisplay::Hidden:  out = Display::NonDisplay; break;
    default: return false;
    }
    return true;
  }


  BOOST_ENUM_VALUES(Capacity, char,
    (Agency)('A')
    (Principal)('P')
    (Riskless)('R')
    (Other)('O')
  );
  inline bool
  convert_from_evt_capacity(const evt::OrderCapacity& in, ouch::Capacity& out) {
    switch(in.index()) {
    case OrderCapacity::Agent:      out = Capacity::Agency; break;
    case OrderCapacity::Principal:  out = Capacity::Principal; break;
    default: return false;
    }
    return true;
  }


  BOOST_ENUM_VALUES(CrossType, char,
    (NoCrossContinuousMarket)('N')
    (OpeningCross)('O')
    (ClosingCross)('C')
    (HaltOrIpoCross)('H')
    (SupplementalOrder)('S')
    (Retail)('R')
  );
  inline bool
  convert_from_evt_ordertype(const evt::OrderType& in, ouch::CrossType& out) {
    switch(in.index()) {
    case OrderType::OnOpen: out = CrossType::OpeningCross; break;
    case OrderType::OnClose: out = CrossType::ClosingCross; break;
    default: out = CrossType::NoCrossContinuousMarket; break;
    }
    return true;
  }


  BOOST_ENUM_VALUES(EventCode, char,
    (StartOfDay)('S')
    (EndOfDay)('E')
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


  inline
  evt::LiquidityCode to_evt_liquidity_code(char in) {
    switch(in) {
    case 'A': return evt::LiquidityCode::AddLit;
    case 'R': return evt::LiquidityCode::RemoveLit;
    case 'k': return evt::LiquidityCode::AddMidpoint;
    case 'm': return evt::LiquidityCode::TakeMidpoint;
    default:  return evt::LiquidityCode::Unknown;
    }
  }


// - Integers are unsigned big-endian
// - Alpha fields are left-justified, padded on the right with spaces
// - Clordids are alphanumeric and case-sensitive and must be day-unique
// - Prices are fixed-point integers with 6 whole number places and 4
//    decimal digits. The maximum price is $199,999.9900 (0x7735939c)
//    and market orders have special price $214,748.3647 (0x7fffffff).
// - Time in force fields are integers


  // NOTE: In order to stay consistent with the other sessions in evts,
  // I'm calling the Ouch Token a 'Clordid'. The fields serve the same
  // purpose, so it shouldn't be too confusing. Just mentally think:
  // s/token/clordid/g.
  struct Clordid {
    char hpr_marker;
    char prefix[5];
    char oid[6];
    char generation[2];

    // helper methods
    /// Overwrites entire Clordid with string (assumes 14 chars)
    void set(const char*);
    /// Sets the prefix (5 chars) and pads to the right if necessary
    bool set_prefix(const std::string&, char padding_char);
    /// Sets oid field to the base36 conversion of the OrderID
    bool set_oid(OrderID);
    /// Adds 1 to the base36 generation
    int increment_generation();

    const std::string str() const;
    int get_generation() const;
    OrderID get_oid() const;
  } __attribute__((packed));


  // SoupBinTCP
  #define MSG_HEADER\
    uint16_t length;\
    char packet_type;

  struct MsgHeader{
    MSG_HEADER
  }__attribute__((packed));

  struct Ouch_MsgHeader{
    MSG_HEADER
    char msg_type;
  }__attribute__((packed));

  // Session-level (SoupbinTCP) messages
  // Inbound (ouch to client)
  // ******************************************************************

  struct LoginAccepted{
    LoginAccepted(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginAccepted)-2))), packet_type(static_cast<char>(PacketType::LoginAccepted)){}
    MSG_HEADER
    char session[10]; // left-padded with spaces
    char seq_num[20]; // left-padded with spaces
  } __attribute__((packed));


  struct LoginRejected{
    LoginRejected(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginRejected)-2))), packet_type(static_cast<char>(PacketType::LoginRejected)){}
    MSG_HEADER
    char reason;
  } __attribute__((packed));

  struct ServerHeartbeat{
    ServerHeartbeat(): length(native_to_big(static_cast<uint16_t>(sizeof(ServerHeartbeat)-2))), packet_type(static_cast<char>(PacketType::ServerHeartbeat)){}
    MSG_HEADER
  } __attribute__((packed));

  struct EndOfSession{
    EndOfSession(): length(native_to_big(static_cast<uint16_t>(sizeof(EndOfSession)-2))), packet_type(static_cast<char>(PacketType::EndOfSession)){}
    MSG_HEADER
  } __attribute__((packed));


  // Outbound (client to ouch)
  // ******************************************************************

  struct LoginRequest{
    LoginRequest(): length(native_to_big(static_cast<uint16_t>(sizeof(LoginRequest)-2))), packet_type(static_cast<char>(PacketType::LoginRequest)){}
    MSG_HEADER
    char username[6]; // right-padded with spaces
    char password[10]; // right-padded with spaces
    char requested_session[10]; // left-padded with spaces
    char requested_seq_num[20]; // left-padded with spaces
  } __attribute__((packed));


  struct ClientHeartbeat{
    ClientHeartbeat(): length(native_to_big(static_cast<uint16_t>(sizeof(ClientHeartbeat)-2))), packet_type(static_cast<char>(PacketType::ClientHeartbeat)){}
    MSG_HEADER
  } __attribute__((packed));


  struct LogoutRequest{
    LogoutRequest(): length(native_to_big(static_cast<uint16_t>(sizeof(LogoutRequest)-2))), packet_type(static_cast<char>(PacketType::LogoutRequest)){}
    MSG_HEADER
  } __attribute__((packed));


  // Application-level (Ouch) messages
  // Outbound (client to ouch)
  // ******************************************************************

  struct EnterOrder{
    EnterOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(EnterOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::EnterOrder)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Clordid clordid;
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
    MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Clordid existing_clordid;
    Clordid clordid;
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
    MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Clordid clordid;
    uint32_t qty; // 0 <= qty <= 1,000,000
  } __attribute__((packed));


  struct ModifyOrder{
    ModifyOrder(): length(native_to_big(static_cast<uint16_t>(sizeof(ModifyOrder)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(OutboundMsgType::ModifyOrder)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::OutboundMsgType
    Clordid clordid;
    char side; ///< \see ouch::Side
    uint32_t qty; // 0 <= qty <= 1,000,000
  } __attribute__((packed));


  // Inbound (ouch to client)
  // ******************************************************************

  struct SystemEvent{
    SystemEvent(): length(native_to_big(static_cast<uint16_t>(sizeof(SystemEvent)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::SystemEvent)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp; // nsecs past midnight
    char event_code; ///< \see ouch::EventCode
  } __attribute__((packed));


  struct OrderAccepted{
    OrderAccepted(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderAccepted)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderAccepted)){}
    void format(){
      timestamp = native_to_big(timestamp);
      price = native_to_big(price);
      qty = native_to_big(qty);
      time_in_force = native_to_big(time_in_force);
      order_reference_number = native_to_big(order_reference_number);
      min_qty = native_to_big(min_qty);
    }
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp; // nsecs past midnight
    Clordid clordid;
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
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderCanceled)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint32_t decrement_qty;
    char reason;
  } __attribute__((packed));


  struct OrderModified{
    OrderModified(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderModified)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderModified)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    char side; ///< \see ouch::Side
    uint32_t shares; // 0 <= shares <= 1,000,000
  } __attribute__((packed));


  struct OrderRejected{
    OrderRejected(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderRejected)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderRejected)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    char reason; ///< \see ouch::RejectedReason
  } __attribute__((packed));


  struct OrderReplaced{
    OrderReplaced(): length(native_to_big(static_cast<uint16_t>(sizeof(OrderReplaced)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::OrderReplaced)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid; ///< called Replacement Token in specs
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
    Clordid orig_clordid; ///< called Previous Token in specs
    char bbo_weight_indicator;
  } __attribute__((packed));


  struct Executed{
    Executed(): length(native_to_big(static_cast<uint16_t>(sizeof(Executed)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::Executed)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint32_t executed_qty;
    int32_t execution_price;
    char liquidity_flag; ///< \see ouch::LiquidityFlag
    uint64_t match_number;
  } __attribute__((packed));


  /// Same as Executed, but with an additional ref price
  struct ExecutedWithRefPrice{
    ExecutedWithRefPrice(): length(native_to_big(static_cast<uint16_t>(sizeof(ExecutedWithRefPrice)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::ExecutedWithRefPrice)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint32_t executed_qty;
    int32_t execution_price;
    char liquidity_flag; ///< \see ouch::LiquidityFlag
    uint64_t match_number;
    uint32_t reference_price;
    char reference_price_type;
  } __attribute__((packed));


  struct BrokenTrade{
    BrokenTrade(): length(native_to_big(static_cast<uint16_t>(sizeof(BrokenTrade)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::BrokenTrade)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint64_t match_number;
    char reason; ///< \see ouch::BrokenTradeReason
  } __attribute__((packed));


  struct CancelPending{
    CancelPending(): length(native_to_big(static_cast<uint16_t>(sizeof(CancelPending)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::CancelPending)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
  } __attribute__((packed));


  struct CancelReject{
    CancelReject(): length(native_to_big(static_cast<uint16_t>(sizeof(CancelReject)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::CancelReject)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
  } __attribute__((packed));


  struct AiqCanceled{
    AiqCanceled(): length(native_to_big(static_cast<uint16_t>(sizeof(AiqCanceled)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::AiqCanceled)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint32_t decrement_qty;
    char reason;
    uint32_t qty_prevented_from_trading;
    uint32_t execution_price;
    char liquidity_flag;
  } __attribute__((packed));


  struct PriorityUpdate{
    PriorityUpdate(): length(native_to_big(static_cast<uint16_t>(sizeof(PriorityUpdate)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::PriorityUpdate)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    int32_t price;
    char display;
    uint64_t order_reference_number;
  } __attribute__((packed));


  struct TradeCorrection{
    TradeCorrection(): length(native_to_big(static_cast<uint16_t>(sizeof(TradeCorrection)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::TradeCorrection)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
    uint32_t executed_shares;
    int32_t execution_price;
    char liquidity_flag;
    uint64_t match_number;
    char reason;
  } __attribute__((packed));


  struct TradeNow{
    TradeNow(): length(native_to_big(static_cast<uint16_t>(sizeof(TradeNow)-2))),
        packet_type(static_cast<char>(PacketType::UnsequencedData)),
        msg_type(static_cast<char>(InboundMsgType::TradeNow)){}
    MSG_HEADER
    char msg_type; ///< \see ouch::InboundMsgType
    uint64_t timestamp;
    Clordid clordid;
  } __attribute__((packed));
} // namespace ouch
} // namespace evt
