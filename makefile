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

install: release
# make release and install to /usr/local/bin/
	@mv $(TARGET) /usr/local/bin

gdb: clean
# for debuging with gdb
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

clean:
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x

uninstall:
	@$(RM) /usr/local/bin/$(TARGET)

depend:
# list deps
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M
