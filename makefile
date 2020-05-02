CC = gcc
RM = rm

IMAGE = image
VIDEO = video
VIDEO2 = video2

ILIBS =
VLIBS = -L/opt/ffmpeg/lib -I/opt/ffmpeg/include/

IFLAGS = -lm
VFLAGS = -lm -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil

all:
	$(CC) $(ILIBS) $(IMAGE).c $(IFLAGS) -o $(IMAGE)
	$(CC) $(VLIBS) $(VIDEO).c $(VFLAGS) -o $(VIDEO)
	$(CC) $(VLIBS) $(VIDEO2).c $(VFLAGS) -o $(VIDEO2)

image:
	$(CC) $(ILIBS) $(IMAGE).c $(IFLAGS) -o $(IMAGE)

video:
	$(CC) $(VLIBS) $(VIDEO).c $(VFLAGS) -o $(VIDEO)

video2:
	$(CC) $(VLIBS) $(VIDEO2).c $(VFLAGS) -o $(VIDEO2)

clean:
	$(RM) $(IMAGE)
	$(RM) $(VIDEO)
