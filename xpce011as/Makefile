EXTRACFLAGS=
OPT_CFLAGS=-O4
MXMLC_DEBUG=false

BUILDDIR = build
SRCDIR = src
INCLUDE := -I$(SRCDIR)
SRCS = $(SRCDIR)/M6502.cpp $(SRCDIR)/Pce.cpp $(SRCDIR)/Sound.cpp $(SRCDIR)/debug.cpp $(SRCDIR)/flash/as3api.cpp $(SRCDIR)/flash/glue.cpp
LIBS = -lAS3++

MODULES = $(SRCS:.c=.o)
MODULES := $(MODULES:.cpp=.o)


TARGET  = xpce
STATICLIB = lib$(TARGET).a
SWC = $(TARGET).swc


.PHONY: all clean

all: $(BUILDDIR) check
	@echo "-------- $(TARGET)"
	make $(BUILDDIR)/$(SWC)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)/$(SRCDIR)
	@mkdir -p $(BUILDDIR)/$(SRCDIR)/flash

$(BUILDDIR)/%.o : %.c
	$(FLASCC)/usr/bin/gcc $(CFLAGS) $(OPT_CFLAGS) $(INCLUDE) -c $< -o $@

$(BUILDDIR)/%.o : %.cpp
	$(FLASCC)/usr/bin/$(CXX) $(CXXFLAGS) $(OPT_CFLAGS) $(INCLUDE) -c $< -o $@

$(BUILDDIR)/$(STATICLIB): $(patsubst %,$(BUILDDIR)/%,$(MODULES))
	cd $(BUILDDIR) && $(FLASCC)/usr/bin/$(AR) r $(STATICLIB) $(MODULES)

$(BUILDDIR)/$(SWC): $(patsubst %,$(BUILDDIR)/%,$(MODULES))
	cd $(BUILDDIR) && $(FLASCC)/usr/bin/g++ $(OPT_CFLAGS) $(LIBS) $(MODULES) -emit-swc=$(TARGET) -o $(SWC)

clean:
	@rm -f -r build

include Makefile.common
