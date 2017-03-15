#pragma once
#include <boost/enum.hpp>


typedef uint64_t OrderID;
constexpr OrderID INVALID_ORDERID = 0;
constexpr OrderID INITIAL_ORDERID = 9100;

BOOST_ENUM(OrderState,
     (INITIAL)
     (NEW)
     (OPEN)
     (PENDING_CANCEL)
     (CANCELED)
     (FILLED)
     (PENDING_REPLACE)
     (REJECTED)
     (IREJECTED)
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
