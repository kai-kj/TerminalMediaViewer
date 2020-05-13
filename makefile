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

#---- no debug flags ----------------------------------------------------------#
release: clean
	@echo "Building..."
	@echo "OS: $(OS)"
ifeq ($(OS), Darwin)
# osx
	@echo "Building for osx..."
	@$(CC) src/$(TARGET).c $(OSXFLAGS) -o $(TARGET)
else ifeq ($(OS), Windows)
# windows
	@echo "Building for windows..."
	@$(CC) src\$(TARGET).c -o $(TARGET)
else
# linux
	@echo "Building for linux..."
	@$(CC) src/$(TARGET).c $(FLAGS) -o $(TARGET)
endif
	@echo "Done"

#---- enable debug flags ------------------------------------------------------#
debug: clean
	@echo "Building with debug flag"
	@$(CC) -Wall src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)
	@echo "Done"

#---- make release and install to /usr/local/bin/ -----------------------------#
install: release
	@echo "Installing..."
	@mv $(TARGET) /usr/local/bin
	@echo "Done"

#---- for debuging with gdb ---------------------------------------------------#
gdb: clean
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

#---- remove local binaries ---------------------------------------------------#
clean:
	@echo "Deleting old binaries..."
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x
	@echo "Done"

#---- remove /usr/local/bin/tmv -----------------------------------------------#
uninstall:
	@echo "Uninstalling..."
	@$(RM) /usr/local/bin/$(TARGET)
	@echo "Done"

#---- list deps ---------------------------------------------------------------#
depend:
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M
