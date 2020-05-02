#define  _POSIX_C_SOURCE 200809L

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG 0

typedef struct VideoInfo
{
	int width;
	int height;
	int fps;
	int frameCount;
	float duration;
}VideoInfo;

typedef struct Pixel
{
	int r;
    int g;
    int b;
}Pixel;

typedef struct Image
{
	int width;
	int height;
	Pixel *pixels;
}Image;

float min(float a, float b)
{
	if(a < b)
		return(a);
	else
		return(b);
}

float getTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	long int time = (long int)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
	float timeF = (float)(time % 10000000) / 1000;

	return(timeF);
}

int comparePixels(const Pixel pixel1, const Pixel pixel2)
{
	if(pixel1.r != pixel2.r || pixel1.g != pixel2.g || pixel1.b != pixel2.b)
		return(1);

	return(0);
}

int getWinWidth()
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return(size.ws_col);
}

int getWinHeight()
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return((size.ws_row - 1) * 2);
}

int getImageX(const char PATH[])
{
	int width, height, components;
	stbi_load(PATH, &width, &height, &components, 3);
	return(width);
}

int getImageY(const char PATH[])
{
	int width, height, components;
	stbi_load(PATH, &width, &height, &components, 3);
	return(height);
}

Image loadImage(const char PATH[])
{
	int width, height, components;

	// Load image
	unsigned char *imageRaw = stbi_load(PATH, &width, &height, &components, 3);

	if(imageRaw == NULL)
	{
		printf("ERROR: invalid image\n");
		exit(1);
	}

	if(DEBUG) printf(
		"DEBUG: loadImage: got raw image (%d, %d)\n", width, height
	);

	Image image;
	image.height = height;
	image.width = width;
	image.pixels = (Pixel*)malloc((width * height) * sizeof(Pixel));

	if(DEBUG) printf("DEBUG: loadImage: allocated memory for image\n");

	// Convert image to better format
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			image.pixels[i * width + j].r = imageRaw[i * width * 3 + j * 3];
			image.pixels[i * width + j].g = imageRaw[i * width * 3 + j * 3 + 1];
			image.pixels[i * width + j].b = imageRaw[i * width * 3 + j * 3 + 2];
		}
	}

	if(DEBUG) printf("DEBUG: loadImage: converted raw image\n");

	free(imageRaw);

	return(image);
}

void updateScreen(Image image, Image prevImage)
{
	int height = image.height;
	int width = image.width;
	for(int i = 0; i < height - 1; i += 2)
	{
		for(int j = 0; j < width - 1; j++)
		{
			#define cPixel1 image.pixels[i * width + j]
			#define cPixel2 image.pixels[(i + 1) * width + j]
			#define pPixel1 prevImage.pixels[i * width + j]
			#define pPixel2 prevImage.pixels[(i + 1) * width + j]

			if(
				comparePixels(image.pixels[i * width + j], prevImage.pixels[i * width + j])
				|| comparePixels(image.pixels[(i + 1) * width + j], prevImage.pixels[(i + 1) * width + j])
			)
			{

				printf("\033[%d;%dH", i / 2 + 1, j + 1);

				printf("\x1b[48;2;%d;%d;%dm", cPixel1.r, cPixel1.g, cPixel1.b);
				printf("\x1b[38;2;%d;%d;%dm", cPixel2.r, cPixel2.g, cPixel2.b);

				printf("▄");
				//printf("\x1b[0m");
			}

		}
	}

	//getchar();
}

VideoInfo getVideoInfo(const char PATH[])
{
	AVFormatContext *formatContext = avformat_alloc_context();
	avformat_open_input(&formatContext, PATH, NULL, NULL);
	avformat_find_stream_info(formatContext,  NULL);

	int streamIndex;

	AVCodecParameters *codecParameters;
	AVCodec *codec;

	for(int i = 0; i < formatContext->nb_streams; i++)
	{
		codec = avcodec_find_decoder(codecParameters->codec_id);
    	codecParameters = formatContext->streams[i]->codecpar;
		if(codecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
			streamIndex = i;
	}

	AVCodecContext *codecCtx = avcodec_alloc_context3(codec);

	avcodec_parameters_to_context(codecCtx, codecParameters);

	VideoInfo info;
	info.width = codecCtx->width;
	info.height = codecCtx->height;
	info.fps = formatContext->streams[streamIndex]->r_frame_rate.num;
	info.duration = formatContext->duration / 1000000;
	info.frameCount = formatContext->streams[streamIndex]->duration;

	free(formatContext);

	return(info);
}

void playVideo(const VideoInfo INFO)
{
	Image prevImage;
	prevImage.pixels = (Pixel*)malloc((INFO.width * INFO.height) * sizeof(Pixel));

	for(int i = 0; i < INFO.height; i++)
	{
		for(int j = 0; j < INFO.width; j++)
		{
			Pixel c;
			c.r = -1;
			c.g = -1;
			c.b = -1;
			prevImage.pixels[i * INFO.width + j] = c;
		}
	}

	float time = getTime();

	for(int i = 0; i < INFO.frameCount; i++)
	{
		char path[1000];
		sprintf(path, "~/.tiv/frame%d.bmp", i + 1);
		Image currentImage = loadImage(path);

		while(getTime() - time < 1 / (float)INFO.fps){}

		time = getTime();
		updateScreen(currentImage, prevImage);
		prevImage = currentImage;
	}
}

void cleanup()
{
	system("rm -r /tmp/tmv.* >>/dev/null 2>>/dev/null");
	system("clear");
	exit(0);
}

int main(const int argc, const char *argv[])
{
	//signal(SIGINT, cleanup);

	char PATH[100];
	sprintf(PATH, "%s", argv[1]);

	VideoInfo info = getVideoInfo(PATH);

	printf("win(%d, %d), img(%d, %d)\n\n", getWinWidth(), getWinHeight(), info.width, info.height);

	float zoom = min(
		(float)getWinWidth() / (float)info.width,
		(float)getWinHeight() / (float)info.height
	);

	printf("%f\n", zoom);
	getchar();

	struct stat st = {0};
	if(stat("~/.tiv", &st) == -1)
	    mkdir("~/.tiv", 0700);

	char command[1000];
	sprintf(
		command,
		"ffmpeg -i %s -vf \"fps=%d, scale=%d:-1\" ~/.tmv/frame%%d.bmp >>/dev/null 2>>/dev/null",
		PATH, info.fps, (int)(info.width * zoom)
	);

	system(command);

	system("clear");
	playVideo(info);

	cleanup();

	return(0);
}
