
GXX := g++
CFLAGS := -g
CXXFLAGS := $(CFLAGS) -std=c++17

ODIR := obj
O := .o
X := .exe
A := .a

XSOURCES := ../unicodestreams.cxx
XOBJS := ../obj/unicodestreams$(O)

$(ODIR)/uni-a$(X): $(ODIR)/uni-a$(O) ../obj/unicodestreams.o
	$(GXX) $(CXXFLAGS) -o $@ $^

$(ODIR)/uni-a$(O): uni-a.cxx ../unicodestreams.hxx
	$(GXX) -c $(CXXFLAGS) -o $@ $<

../obj/unicodestreams$(O): ../unicodestreams.cxx ../unicodestreams.hxx
	$(GXX) -c $(CXXFLAGS) -o $@ $<

