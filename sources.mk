TARGETS := evtsim #regtest

INCLUDES :=

SOURCES := evtsim_main.cpp evtsim_util.cpp ouch_session.cpp order.cpp

REGTEST_SOURCES=$(filter-out evtsim_main.cpp, $(SOURCES)) regtest.cpp
