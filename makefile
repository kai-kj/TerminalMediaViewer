#---- colors ------------------------------------------------------------------#

BLACK        := $(shell tput -Txterm setaf 0)
RED          := $(shell tput -Txterm setaf 1)
GREEN        := $(shell tput -Txterm setaf 2)
YELLOW       := $(shell tput -Txterm setaf 3)
LIGHTPURPLE  := $(shell tput -Txterm setaf 4)
PURPLE       := $(shell tput -Txterm setaf 5)
BLUE         := $(shell tput -Txterm setaf 6)
WHITE        := $(shell tput -Txterm setaf 7)
RESET		 := $(shell tput -Txterm sgr0)

#---- get info ----------------------------------------------------------------#
OS = $(shell uname)
SFLAG = $(shell id -u)

ifeq ($(OS), )
	OS = Windows
endif

#---- set commands ------------------------------------------------------------#

CC = gcc

ifeq ($(OS), Windows)
	RM = del /f
else
	RM = rm -f
endif

#---- set target and flags ----------------------------------------------------#

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
	@$(CC) -w src/$(TARGET).c $(OSXFLAGS) -o $(TARGET)
else ifeq ($(OS), Windows)
# windows
	@echo "Building for windows..."
	@$(CC) -w src\$(TARGET).c -o $(TARGET)
else
# linux
	@echo "Building for linux..."
	@$(CC) -w src/$(TARGET).c $(FLAGS) -o $(TARGET)
endif
	@echo "$(GREEN)DONE$(RESET)"

ifeq (, $(@shell which ffmpeg))
	@echo "$(YELLOW)NOTE$(RESET): ffmpeg is not installed (can't play videos)"
endif

ifeq (, $(@shell which youtube-dl))
	@echo "$(YELLOW)NOTE$(RESET): youtube-dl is not installed (can't download videos)"
endif

#---- enable debug flags ------------------------------------------------------#
debug: clean
	@echo "Building with debug flag"
	@$(CC) -Wall src/$(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)
	@echo "$(GREEN)DONE$(RESET)"

#---- make release and install to /usr/local/bin/ -----------------------------#
install: uninstall release
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Installing..."
	@mv $(TARGET) /usr/local/bin
	@echo "$(GREEN)DONE$(RESET)"
else
	@echo "$(RED)ERROR$(RESET): sudo privileges needed for install"
endif

#---- remove /usr/local/bin/tmv -----------------------------------------------#
uninstall:
# check if sudo
ifeq ($(SFLAG), 0)
	@echo "Uninstalling..."
	@$(RM) /usr/local/bin/$(TARGET)
	@echo "$(GREEN)DONE$(RESET)"
else
	@echo "$(RED)ERROR$(RESET): sudo privileges needed for uninstall"
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
	@echo "$(GREEN)DONE$(RESET)"
