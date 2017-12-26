
GXX := g++
CFLAGS := -g
CXXFLAGS := $(CFLAGS) -std=c++17

ODIR := obj
O := .o
X := .exe
A := .a

SOURCES := unicodestreams.cxx

$(ODIR)/unicodestreams$(O): unicodestreams.cxx unicodestreams.hxx
	$(GXX) -c $(CXXFLAGS) -o $@ $<

