CC = gcc
RM = rm

IMAGE = image
VIDEO = video

ILIBS =
VLIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

IFLAGS = -lm
VFLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil

all:
	$(CC) $(ILIBS) $(IMAGE).c $(IFLAGS) -o $(IMAGE)
	$(CC) $(VLIBS) $(VIDEO).c $(VFLAGS) -o $(VIDEO)

image:
	$(CC) $(ILIBS) $(IMAGE).c $(IFLAGS) -o $(IMAGE)

video:
	$(CC) $(VLIBS) $(VIDEO).c $(VFLAGS) -o $(VIDEO)

clean:
	$(RM) $(IMAGE)
	$(RM) $(VIDEO)
