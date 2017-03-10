EVTCORE_VERSION=1.67
TBB_VERSION=20160916oss

LDFLAGS=-ldl

# evtcore
ifeq ($(EVTCORE_VERSION),local)
  EVTCORE=../evtcore
  include $(EVTCORE)/thirdparty.mk
  CXXFLAGS+=-isystem$(EVTCORE)
  LDFLAGS+=-L$(EVTCORE) -levtcore -Wl,-rpath -Wl,$(abspath $(EVTCORE))
else
  EVTCORE=/evt/apps/evtcore/$(EVTCORE_VERSION)
  include $(EVTCORE)/build/thirdparty.mk
  CXXFLAGS+=-isystem$(EVTCORE)/include
  LDFLAGS+=-L$(EVTCORE)/lib -levtcore -Wl,-rpath -Wl,$(EVTCORE)/lib
endif

# boost
LDFLAGS+=-lboost_program_options -lboost_date_time

# tbb
TBB=/evt/apps/tbb/$(TBB_VERSION)
CXXFLAGS+=-isystem$(TBB)/include
LDFLAGS+=-L$(TBB)/lib -Wl,-rpath -Wl,$(TBB)/lib -ltbb
