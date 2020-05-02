#define  _POSIX_C_SOURCE 200809L

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG 0

typedef struct VideoInfo
{
	char path[100];
	char dir[100];
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

int getWinX()
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return(size.ws_col);
}

int getWinY()
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

Image loadImage(const char PATH[], const float ZOOM)
{
	int width, height, components;

	// Load image
	unsigned char *rawImage = stbi_load(PATH, &width, &height, &components, 3);

	if(rawImage == NULL)
	{
		printf("ERROR: invalid image\n");
		exit(1);
	}

	if(DEBUG) printf(
		"DEBUG: loadImage: got raw image (%d, %d)\n", width, height
	);

	Image image;
	image.width = (int)(width * ZOOM);
	image.height = (int)(height * ZOOM);
	float pixelWidth = 1 / ZOOM;

	image.pixels
		= (Pixel*)malloc((image.width * image.height) * sizeof(Pixel));

	for(int i = 0; i < image.height; i++)
	{
		for(int j = 0; j < image.width; j++)
		{
			#define pixel image.pixels[i * image.width + j]
			pixel.r = 0;
			pixel.g = 0;
			pixel.b = 0;
			int count = 0;

			for(float k = 0; k < pixelWidth; k += 0.1)
			{
				int xPos = (int)floor(j * pixelWidth + k);
				int yPos = (int)floor(i * pixelWidth + k);

				pixel.r += rawImage[yPos * width * 3 + xPos * 3];
				pixel.g += rawImage[yPos * width * 3 + xPos * 3 + 1];
				pixel.b += rawImage[yPos * width * 3 + xPos * 3 + 2];
				count++;
			}

			pixel.r = (int)((float)pixel.r / (float)count);
			pixel.g = (int)((float)pixel.g / (float)count);
			pixel.b = (int)((float)pixel.b / (float)count);

			if
			(
				pixel.r > 255 || pixel.g > 255 || pixel.b > 255
				|| pixel.r < 0 || pixel.g < 0 || pixel.b < 0
			)
			{
				printf
				(
					"ERROR: invalid color value (%d, %d, %d) @(%d, %d)\n",
					pixel.r, pixel.g, pixel.b, j, i
				);
				exit(1);
			}
		}
	}
	return(image);
}

void displayImage(Image image)
{
	int height = image.height;
	int width = image.width;
	for(int i = 0; i < height - 1; i += 2)
	{
		for(int j = 0; j < width - 1; j++)
		{
			#define pixel1 image.pixels[i * width + j]
			#define pixel2 image.pixels[(i + 1) * width + j]

			printf("\x1b[48;2;%d;%d;%dm", pixel1.r, pixel1.g, pixel1.b);
			printf("\x1b[38;2;%d;%d;%dm", pixel2.r, pixel2.g, pixel2.b);

			printf("▄");
			printf("\x1b[0m");
		}

		printf("\n");
	}
}

VideoInfo getVideoInfo(const char PATH[])
{
	AVFormatContext *formatContext = avformat_alloc_context();
	avformat_open_input(&formatContext, PATH, NULL, NULL);
	avformat_find_stream_info(formatContext,  NULL);

	int streamIndex;

	for(int i = 0; i < formatContext->nb_streams; i++)
	{
		AVCodecParameters *localCodecParameters = formatContext->streams[i]->codecpar;
		if(localCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
			streamIndex = i;
	}

	VideoInfo info;
	sprintf(info.path, "%s", PATH);
	info.fps = formatContext->streams[streamIndex]->r_frame_rate.num;
	info.duration = formatContext->duration / 1000000;
	info.frameCount = formatContext->streams[streamIndex]->duration;

	free(formatContext);

	return(info);
}

char *openVideo(const VideoInfo INFO)
{
	char command[1000];

	char template[] = "/tmp/tmv.XXXXXX";
	char *dir = malloc(500);

	sprintf(dir, "%s", mkdtemp(template));

	sprintf(
		command,
		"ffmpeg -i %s -vf fps=%d %s/frame%%d.bmp >>/dev/null 2>>/dev/null",
		INFO.path, INFO.fps, dir
	);

	system(command);

	return(dir);

}

void playVideo(const VideoInfo INFO, const char *DIR, const int WINX, const int WINY)
{
	char path[100];
	sprintf(path, "%s/frame1.bmp", DIR);

	float zoom = min(
		(float)WINX / (float)getImageX(path),
		(float)WINY / (float)getImageY(path)
	);

	for(int i = 0; i < INFO.frameCount; i++)
	{
		sprintf(path, "%s/frame%d.bmp", DIR, i + 1);
		Image image = loadImage(path, zoom);
		system("clear");
		displayImage(image);
	}
}

void cleanup()
{
	system("rm -r /tmp/tmv.* >>/dev/null 2>>/dev/null");
	exit(0);
}

int main(const int argc, const char *argv[])
{
	signal(SIGINT, cleanup);

	char PATH[100];
	sprintf(PATH, "%s", argv[1]);

	int winX = getWinX();
	int winY = getWinY();

	VideoInfo info = getVideoInfo(PATH);

	char *dir = openVideo(info);

	playVideo(info, dir, winX, winY);

	cleanup();

	return(0);
}
