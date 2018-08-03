SHELL := bash

CC := gcc
CXX := g++
CFLAGS = -Wall
CXXFLAGS = -Wall

TARGET = base64

SRCS :=
SRCS += Base64.cpp
SRCS += main.cpp

OBJS := $(patsubst %.cpp, %.o, $(SRCS))

all : $(TARGET)

.PHONY: all clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	@rm $(TARGET) || [ 1 == 1 ];
	@rm $(OBJS) || [ 1 == 1 ];
