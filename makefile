CC = gcc
RM = rm -f

TARGET = tmv

LIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl

release: clean
	# no debug flags
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET)

debug: clean
	# enable debug flags
	@$(CC) -Wall $(LIBS) src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

gdb: clean
	# for debuging with gdb
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

clean:
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x

depend:
	# list deps
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M
