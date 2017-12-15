ALL_DIR_SRC 	= ./
INCLUDES 	= -Icoroutine
CPPFLAGS	= -pthread -gdwarf-2

all : project

include Makefile.inc

project : $(ALL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^
