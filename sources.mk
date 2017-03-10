TARGETS := evtsim regtest

INCLUDES := 

SOURCES := evtsim_main.cpp

REGTEST_SOURCES=$(filter-out evtsim_main.cpp, $(SOURCES)) regtest.cpp
