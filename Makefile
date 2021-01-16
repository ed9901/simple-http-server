SOURCES=httpserver.cpp getrequest.cpp putrequest.cpp
INCLUDES=$(wildcard *.h)
TARGET=httpserver
CXXFLAGS=-std=gnu++11 -Wall -Wextra -Wpedantic -Wshadow -g -Og
LDFLAGS= -pthread
_submit_CXXFLAGS=-std=gnu++11 -Wall -Wextra -Wpedantic -Wshadow -g -O2
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
CXX=clang++
all: $(TARGET)

clean:
	-rm $(DEPS) $(OBJECTS)

spotless: clean
	-rm $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -MD -o $@ $<
-include $(DEPS)
.PHONY: all clean format spotless
