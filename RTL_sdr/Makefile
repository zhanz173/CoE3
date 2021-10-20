# this makefile is intended for g++ on Linux

CC = g++
CFLAGS = -std=c++17 -c -O3 -Wall -Wpedantic -pthread
CXXFLAG = -std=c++17 -O3 -Wall -Wpedantic -pthread
LDFLAGS =  -pthread -O3
SOURCES = filter.cpp genfunc.cpp  main.cpp PLL.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = experiment

all: $(EXECUTABLE) clean

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm $(OBJECTS)
