CC = arm-poky-linux-gnueabi-gcc
CFLAGS = -g -Wall -std=c++11 -shared-libgcc -I.
LIBS   = -lopencv_video -lopencv_core -lopencv_highgui \
	 -lopencv_imgproc -lstdc++ -lpthread -llttng-ust \
	 -ldl
FILES = trace.c opencv.cpp 
TARGET = evision

all: $(FILES)
	$(CC) $(CFLAGS) $(LIBS) -o $(TARGET) $(FILES)

clean:
	rm $(TARGET)

