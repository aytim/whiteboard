CC=g++
CPPFLAGS=-O3
LFLAGS=-lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

SRC=\
	quad.cpp \
	huff.cpp \
	bitwriter.cpp \
	bitreader.cpp

all: wb unwb

wb: comp.cpp
	$(CC) comp.cpp $(SRC) $(CPPFLAGS) -o $@ $(LFLAGS)

unwb: decomp.cpp
	$(CC) decomp.cpp $(SRC) $(CPPFLAGS) -o $@ $(LFLAGS)
