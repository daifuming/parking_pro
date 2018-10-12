CC=arm-linux-gcc
TARGET=main
#SRCS=$(wildcard *.c)
#OBJS=$(patsubst %.c, %.o, $(SRCS))
OBJS=$(patsubst %.c, %.o, $(wildcard *.c))
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lm -lpthread -ldl

clean:
	rm $(OBJS) $(TARGET)