OS = $(shell uname)
SFLAG = $(shell id -u)

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
install: release uninstall
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Installing..."
	@mv $(TARGET) /usr/local/bin
	@echo "Done"
else
	@echo "\nERROR: sudo privileges needed for install\n"
endif

#---- remove /usr/local/bin/tmv -----------------------------------------------#
uninstall:
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Uninstalling..."
	@$(RM) /usr/local/bin/$(TARGET)
	@echo "Done"
else
	@echo "\nERROR: sudo privileges needed for uninstall\n"
endif

#---- for debuging with gdb ---------------------------------------------------#
gdb: clean
	@$(CC) -g $(LIBS) src/$(TARGET).c $(FLAGS) -o $(TARGET).x

#---- list deps ---------------------------------------------------------------#
depend:
	@$(CC) $(LIBS) src/$(TARGET).c $(FLAGS) -M

#---- remove local binaries ---------------------------------------------------#
clean:
	@echo "Deleting old binaries..."
	@$(RM) $(TARGET)
	@$(RM) $(TARGET).x
	@echo "Done"