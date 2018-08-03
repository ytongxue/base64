SHELL := bash

CC := gcc
CXX := g++

TARGET = base64

SRCS :=
SRCS += Base64.cpp
SRCS += main.cpp

OBJS := $(patsubst %.cpp, %.o, $(SRCS))

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^
