#pragma once
#include <boost/enum.hpp>
#include <time.h>

typedef uint64_t OrderID;
constexpr OrderID INVALID_ORDERID = 0;
constexpr OrderID INITIAL_ORDERID = 9100;

constexpr uint32_t IOC_time = 0;
constexpr uint32_t market_hours = 99998;
constexpr uint32_t system_hours = 99999;

namespace evt{
namespace ouch{

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

BOOST_ENUM(OrderTIF,
     (Day)
     (IOC)
     (FillOrKill)
     (PostOnly)
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

BOOST_ENUM_VALUES(OrderSide,char,
		    (Invalid)('U')
		    (Buy)('B')
		    (Sell)('S')
		    (Short)('T')
		    );

class order{
public:
  order();
  order(EnterOrder & eo);
  void parse_order(EnterOrder & eo);
  bool still_live();

  time_t recv_order_time;
  uint32_t time_in_force;
  int32_t remain_time_in_force;
  uint32_t qty;
  int32_t price;
  uint32_t min_qty;
  char symbol[8];
  Token token;
  OrderState state;
  Side side;
};

}}
