ARCH=$(shell uname -m)
SRCDIR=$(shell pwd -L)
BUILDMODE=debug
INSTALL_ROOT = $(SRCDIR)
INSTALL_DIR = $(INSTALL_ROOT)/.install
VERSION ?= dev

include thirdparty.mk
include sources.mk

# speedup
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

CXXFLAGS+=-MMD -I$(SRCDIR) -DEVTSIM_VERSION=\"$(VERSION)\"
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)
REGTEST_OBJECTS=$(REGTEST_SOURCES:.cpp=.o)

ifeq ($(BUILDMODE),debug)
  CXXFLAGS+=-g
  LDFLAGS+=-g
endif

ifeq ($(BUILDMODE),opt)
  CXXFLAGS+=-O2 -g -DBOOST_DISABLE_ASSERTS
  LDFLAGS+=-O2 -g
endif

ifeq ($(VERSION),dev)
  RPATH_ADD=-L$(SRCDIR) -Wl,-rpath -Wl,$(SRCDIR)
else
  RPATH_ADD=-L$(SRCDIR) -L$(INSTALL_DIR)/lib -Wl,-rpath -Wl,$(INSTALL_DIR)/lib
endif

all: $(TARGETS)

dep: $(DEPENDS)

clean:
	rm -f $(TARGETS) $(OBJECTS) $(DEPENDS) regtest.o

%.o: %.cpp
	$(CXX) -c -fPIC $(CXXFLAGS) $< -o $@

evtsim: $(OBJECTS) evtsim_main.o
	$(CXX) -fPIC $(OBJECTS) $(LDFLAGS) -o $@

# regtest: $(REGTEST_OBJECTS)
# 	$(CXX) -fPIC $(REGTEST_OBJECTS) $(LDFLAGS) $(RPATH_ADD) -o $@ -lpthread

install:
	for elem in include bin lib build; do install -d -m 0755 $(INSTALL_DIR)/$$elem; done
	install -m 0444 $(SRCDIR)/$(INCLUDES) $(INSTALL_DIR)/include
	install -m 0555 $(SRCDIR)/evtsim $(INSTALL_DIR)/bin
	install -m 0444 $(SRCDIR)/thirdparty.mk $(INSTALL_DIR)/build

print-% : ; @echo $* = $($*)

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPENDS)
endif
