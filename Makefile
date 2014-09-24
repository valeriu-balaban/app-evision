CC = arm-poky-linux-gnueabi-gcc

CFLAGS = -g -Wall -std=c++11 -shared-libgcc

LIBS   = -lopencv_video -lopencv_core -lopencv_highgui \
	 -lopencv_imgproc -lstdc++ -lpthread 

FILES = evision.cpp 
TARGET = evision

all: $(FILES)
	$(CC) $(CFLAGS) $(LIBS) -o $(TARGET) $(FILES)

clean:
	rm $(TARGET)

