OS = $(shell uname)

ifeq ($(OS), )
	OS = Windows
endif

CC = gcc

ifeq ($(OS), Windows)
	RM = del /f
else
	RM = rm -f
endif

TARGET = tmv

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl
OSXFLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl -largp

release: clean
# no debug flags
	@echo "Building..."
	@echo "OS: $(OS)"
ifeq ($(OS), Darwin)
# osx
	@$(CC) src/$(TARGET).c $(OSXFLAGS) -o $(TARGET)
else ifeq ($(OS), Windows)
# windows
	@$(CC) src\$(TARGET).c -o $(TARGET)
# linux
	@$(CC) src/$(TARGET).c $(FLAGS) -o $(TARGET)
endif
	@echo "Done"

debug: clean
# enable debug flags
	@$(CC) -Wall src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

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
