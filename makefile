CC = gcc
RM = rm -f

TARGET = tmv

LIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

FLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lpthread -ldl

release: clean
	@$(CC) $(LIBS) $(TARGET).c $(FLAGS) -o $(TARGET)

debug: clean
	@$(CC) $(LIBS) $(TARGET).c $(FLAGS) -D DEBUG -o $(TARGET)

clean:
	@$(RM) $(TARGET)

depend:
	@$(CC) $(LIBS) $(TARGET).c $(FLAGS) -M
