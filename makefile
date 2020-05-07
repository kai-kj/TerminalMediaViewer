CC = gcc
RM = rm -f

TARGET = tmv

LIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl

release: clean
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET)

debug: clean
	@$(CC) -Wall $(LIBS) src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

gdb: clean
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

clean:
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x

depend:
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M
