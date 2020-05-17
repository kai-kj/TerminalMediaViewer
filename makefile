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

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl -largp
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
	@echo "\e[32mDONE\e[39m"

ifeq (, $(shell which ffmpeg))
	@echo "\e[33mNOTE\e[39m: ffmpeg is not installed (can't play videos)"
endif

ifeq (, $(shell which youtube-dl))
	@echo "\e[33mNOTE\e[39m: youtube-dl is not installed (can't download videos)"
endif

#---- enable debug flags ------------------------------------------------------#
debug: clean
	@echo "Building with debug flag"
	@$(CC) -Wall src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)
	@echo "\e[32mDONE\e[39m"

#---- make release and install to /usr/local/bin/ -----------------------------#
install: uninstall release
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Installing..."
	@mv $(TARGET) /usr/local/bin
	@echo "\e[32mDONE\e[39m"
else
	@echo "\e[31mERROR\e[39m: sudo privileges needed for install"
endif

#---- remove /usr/local/bin/tmv -----------------------------------------------#
uninstall:
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Uninstalling..."
	@$(RM) /usr/local/bin/$(TARGET)
	@echo "\e[32mDONE\e[39m"
else
	@echo "\e[31mERROR\e[39m: sudo privileges needed for uninstall"
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
	@echo "\e[32mDONE\e[39m"
