CXXFLAGS = \
					 -I/usr/local/include

LDFLAGS = \
					-L/usr/local/lib \
					-lopencv_core \
					-lopencv_highgui \
					-lopencv_imgproc \
					-lopencv_legacy

all:
	     g++ -o manga.out $(CXXFLAGS) $(LDFLAGS) manga.cpp
