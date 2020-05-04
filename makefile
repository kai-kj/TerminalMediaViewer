CC = gcc
RM = rm -f

TARGET = tmv

LIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil

debug: clean
	@$(CC) $(LIBS) $(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

release: clean
	@$(CC) $(LIBS) $(TARGET).c $(FLAGS) -o $(TARGET)

test: debug
	@clear
	@echo "---- image test (press ENTER) ----"
	@read
	./$(TARGET) tests/iTest.*
	@echo "---- (press ENTER) ----"
	@read
	@clear
	@echo "---- video test (press ENTER) ----"
	@read
	./$(TARGET) tests/vTest.*

test-release: release
	@clear
	@echo "---- image test (press ENTER) ----"
	@read
	@./$(TARGET) tests/iTest.*
	@echo "---- (press ENTER) ----"
	@read
	@clear
	@echo "---- video test (press ENTER) ----"
	@read
	@./$(TARGET) tests/vTest.*

clean:
	@$(RM) $(TARGET)
