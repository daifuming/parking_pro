CC=arm-linux-gcc
TARGET=main
OBJS=$(patsubst %.c, %.o, $(wildcard *.c))
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lm -lpthread -ldl

clean:
	rm $(OBJS) $(TARGET)