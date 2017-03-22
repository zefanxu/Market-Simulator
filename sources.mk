TARGETS := evtsim #regtest

INCLUDES :=

SOURCES := evtsim_main.cpp evtsim_util.cpp session.cpp order.cpp tcp_server.cpp

REGTEST_SOURCES=$(filter-out evtsim_main.cpp, $(SOURCES)) regtest.cpp
