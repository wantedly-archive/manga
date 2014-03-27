CXXFLAGS = \
					 -I/usr/local/include

LDFLAGS = \
					-L/usr/local/lib \
					-lopencv_core \
					-lopencv_highgui \
					-lopencv_features2d \
					-lopencv_imgproc \
					-lopencv_nonfree \
					-lopencv_legacy

all:
	     g++ $(CXXFLAGS) $(LDFLAGS) main.cpp
