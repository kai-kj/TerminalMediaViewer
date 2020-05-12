CC = gcc
RM = rm -f

TARGET = tmv

LIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/
FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl
OSXFLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl -largp

OS = $(shell uname)

release: clean
# no debug flags
	@echo "Building..."
	@echo "OS: $(OS)"
ifeq ($(OS), Darwin)
# osx
	@$(CC) src/$(TARGET).c $(OSXFLAGS) -o $(TARGET)
else
# linux
	@$(CC) src/$(TARGET).c $(FLAGS) -o $(TARGET)
endif
	@echo "Done"

debug: clean
# enable debug flags
	@$(CC) -Wall $(LIBS) src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

install: release
# make release and install to /usr/local/bin/
	@echo "Installing..."
	@mv $(TARGET) /usr/local/bin
	@echo "Done"

gdb: clean
# for debuging with gdb
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

clean:
	@echo "Deleting old binaries..."
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x
	@echo "Done"

uninstall:
	@echo "Uninstalling..."
	@$(RM) /usr/local/bin/$(TARGET)
	@echo "Done"

depend:
# list deps
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M
