//============================================================================//
// Terminal Media Viewer
//============================================================================//

/*
v0.1 - Initial release

* View images
* Watch videos (with sound)
* Resize images / videos

*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Licence
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/*
MIT License

Copyright (c) 2020 Kai Kitagawa-Jones

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Includes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#define  _POSIX_C_SOURCE 200809L

//-------- standard libraries ------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <argp.h>

//-------- POSIX libraries ---------------------------------------------------//

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include<sys/wait.h>

//-------- ffmpeg ------------------------------------------------------------//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

//-------- external libraries ------------------------------------------------//

// reading images <https://github.com/nothings/stb>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

// playing audio files <https://github.com/dr-soft/miniaudio>
#define DR_WAV_IMPLEMENTATION
#include "include/dr_wav.h"

#define MINIAUDIO_IMPLEMENTATION
#include "include/miniaudio.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Image / Video format lists
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "formatList.h"
char vFormats[][25];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Defines
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#define DEFAULT_FPS 15
#define TMP_FOLDER "/tmp/tmv"

// number of samples to take when scaling (bigger -> better but slow)
// 1 = nearest neighbor
#define SCALE 5

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Types
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

typedef struct VideoInfo
{
	int width;
	int height;
	int fps;
	int frameCount;
}VideoInfo;

typedef struct Pixel
{
	unsigned short r;
    	unsigned short g;
    	unsigned short b;
}Pixel;

typedef struct Image
{
	int width;
	int height;
	Pixel *pixels;
}Image;

void freeImage(Image *image)
{
	if(image->pixels != NULL) free(image->pixels);
	image->pixels = NULL;
}

Image copyImage(Image image)
{
	Image newImage;
	newImage.width = image.width;
	newImage.height = image.height;

	newImage.pixels = malloc((image.width * image.height) * sizeof(Pixel));

	for(int i = 0; i < image.height; i++)
	{
		for(int j = 0; j < image.width; j++)
		{
			int index = i * image.width + j;
			newImage.pixels[index].r = image.pixels[index].r;
			newImage.pixels[index].g = image.pixels[index].g;
			newImage.pixels[index].b = image.pixels[index].b;
		}
	}
	return(newImage);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Debug
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int debugFunc(const char *FUNC, const char *FMT, ...)
{
	#ifdef DEBUG
		char msg[4096];
	    va_list args;
	    va_start(args, FMT);
	    vsnprintf(msg, sizeof(msg), FMT, args);
		printf(
			"\e[33mDEBUG\e[39m(\e[32m%s\e[39m) %s\n"
			"\e[90m[press ENTER to continue...]\e[39m",
			FUNC, msg
		);
		getchar();
	    va_end(args);
	    return 0;
	#else
		return 1;
	#endif
}

#define debug(a, ...) debugFunc(__func__, (a), ##__VA_ARGS__)

void errorFunc(const char *FUNC, const char *FMT, ...)
{
	char msg[4096];
    va_list args;
    va_start(args, FMT);
    vsnprintf(msg, sizeof(msg), FMT, args);
	printf("\e[31mERROR\e[39m(\e[32m%s\e[39m): %s\n", FUNC, msg);
    va_end(args);
	exit(1);
}

#define error(a, ...) errorFunc(__func__, (a), ##__VA_ARGS__)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Argp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

const char *argp_program_version = "tmv v0.1";

const char *argp_program_bug_address = "<kal390983@gmail.com>";

char doc[] =
	"\nView images and videos without leaving the console.\n"
	"Requires a terminal that supports truecolor and utf-8\n"
	"For more info visit <https://github.com/kal39/TerminalMediaViewer>";

char args_doc[] = "[INPUT FILE]";

static struct argp_option options[] = {
    {"width", 'w', "[width]", 0, "Set output width.", 1},
	{"height", 'h', "[height]", 0, "Set output height.", 1},
	{"fps", 'f', "[target fps]", 0, "Set target fps. Default 15 fps", 2},
	{"origfps", 'F', 0, 0, "Use original fps from video. Default 15 fps", 2},
	{"no-sound", 's', 0, 0, "disable sound", 3},
	{ 0 }
};

struct args {
	char *input;
	int width;
	int height;
	int fps;
	int fpsFlag;
	int sound;
};

static error_t parse_option(int key, char *arg, struct argp_state *state)
{
	struct args *args = state->input;
	switch(key)
	{
		case ARGP_KEY_ARG:
			if(state->arg_num == 0)
				args->input = arg;
			else
				argp_usage( state );
			break;
		case 'f':
			if(atoi(arg) <= 0) error("invalid fps value");
			args->fps = atoi(arg);
			break;
		case 'F':
			args->fpsFlag = 1;
			break;
		case 'w':
			if(atoi(arg) <= 0) error("invalid width value");
			args->width = atoi(arg);
			break;
		case 'h':
			if(atoi(arg) <= 0) error("invalid height value");
			args->height = atoi(arg);
			break;
		case 's':
			args->sound = 0;
			break;
		case ARGP_KEY_END:
			if(args->input == NULL)
				argp_usage( state );
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

struct argp argp = {
	options,
	parse_option,
	args_doc,
	doc
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Misc
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

float min(float a, float b)
{
	if(a < b) return(a);
	else return(b);
}

float getTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long int time = (long int)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
	return((float)(time % 10000000) / 1000);
}

// get file extension
char *getExtension(const char *TARGET)
{
    char *dot = strrchr(TARGET, '.');
    if(!dot || dot == TARGET) return "";
    else return dot + 1;
}

int checkFileType(const char EXT[])
{
	int fileType = -1;
	for(int i = 0; ; i++)
	{
		if(strcasecmp(EXT, vFormats[i]) == 0)
		{
			fileType = 2;
			break;
		}
		else if(strcmp("END", vFormats[i]) == 0)
			break;

	}

	for(int i = 0; ; i++)
	{
		if(strcasecmp(EXT, iFormats[i]) == 0)
		{
			fileType = 1;
			break;
		}
		else if(strcmp("END", iFormats[i]) == 0)
			break;

	}

	return(fileType);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Window
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

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
	return((size.ws_row) * 2);
}

// system("clear") doesn't let you scroll back
void clear()
{
	for(int i = 0; i < getWinHeight(); i++)
		printf("\n");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Screen
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void displayImage(Image image)
{
	for(int i = 0; i < image.height - 1; i += 2) // update 2 pixels at once
	{
		for(int j = 0; j < image.width - 1; j++)
		{
			#define pixel1 image.pixels[i * image.width + j]
			#define pixel2 image.pixels[(i + 1) * image.width + j]

			// move cursor
			printf("\033[%d;%dH", i / 2 + 1, j + 1);

			// set foreground and background colors
			printf("\x1b[48;2;%d;%d;%dm", pixel1.r, pixel1.g, pixel1.b);
			printf("\x1b[38;2;%d;%d;%dm", pixel2.r, pixel2.g, pixel2.b);

			printf("▄");

			// reset colors
			printf("\x1b[0m");
		}
	}
}

// only updates changed pixels (-> faster than displayImage())
void updateScreen(Image image, Image prevImage)
{
	for(int i = 0; i < image.height - 1; i += 2) // update 2 pixels at once
	{
		for(int j = 0; j < image.width - 1; j++)
		{
			#define cPixel1 image.pixels[i * image.width + j]
			#define cPixel2 image.pixels[(i + 1) * image.width + j]
			#define pPixel1 prevImage.pixels[i * prevImage.width + j]
			#define pPixel2 prevImage.pixels[(i + 1) * prevImage.width + j]

			// draw only if pixel has changed
			if(
				cPixel1.r != pPixel1.r ||
				cPixel1.g != pPixel1.g ||
				cPixel1.b != pPixel1.b ||
				cPixel2.r != pPixel2.r ||
				cPixel2.g != pPixel2.g ||
				cPixel2.b != pPixel2.b
			)
			{
				// move cursor
				printf("\033[%d;%dH", i / 2 + 1, j + 1);

				// set foreground and background colors
				printf("\x1b[48;2;%d;%d;%dm", cPixel1.r, cPixel1.g, cPixel1.b);
				printf("\x1b[38;2;%d;%d;%dm", cPixel2.r, cPixel2.g, cPixel2.b);

				printf("▄");
			}
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Audio
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}

ma_decoder decoder;
ma_device device;

void playAudio(const char PATH[])
{
    ma_result result;
    ma_device_config deviceConfig;

    result = ma_decoder_init_file(PATH, NULL, &decoder);

	if(result != MA_SUCCESS) error("could not initialize deoder");

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    result = ma_device_init(NULL, &deviceConfig, &device);

	if(result != MA_SUCCESS) error("could not initialize deoder");

    result = ma_device_start(&device);

	if(result != MA_SUCCESS) error("could not initialize deoder");
}

void stopAudio()
{
	ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Image
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int getImageWidth(const char TARGET[])
{
	int width = -1;
	stbi_load(TARGET, &width, NULL, NULL, 3);
	return(width);
}

int getImageHeight(const char TARGET[])
{
	int height = -1;
	stbi_load(TARGET, NULL, &height, NULL, 3);
	return(height);
}

Image loadImage(const char TARGET[])
{
	Image image;

	unsigned char *imageRaw = stbi_load(
		TARGET, &image.width, &image.height, NULL, 3
	);

	if(imageRaw == NULL)
		error("could not open %s (it may be corrupt)", TARGET);

	image.pixels = (Pixel*)malloc((image.width * image.height) * sizeof(Pixel));

	if(image.pixels == NULL)
		error("failed to allocate memory for image");

	//debug("allocated mempry for image");

	// Convert to "Image" type (easier to use)
	for(int i = 0; i < image.height; i++)
	{
		for(int j = 0; j < image.width; j++)
		{
			image.pixels[i * image.width + j].r
				= imageRaw[i * image.width * 3 + j * 3];
			image.pixels[i * image.width + j].g
				= imageRaw[i * image.width * 3 + j * 3 + 1];
			image.pixels[i * image.width + j].b
				= imageRaw[i * image.width * 3 + j * 3 + 2];
		}
	}

	free(imageRaw);
	return(image);
}

Image scaleImage(Image oldImage, float zoomX, float zoomY)
{
	Image newImage;
	newImage.width = (int)(oldImage.width * zoomX);
	newImage.height = (int)(oldImage.height * zoomY);
	float xPixelWidth = 1 / zoomX;
	float yPixelWidth = 1 / zoomY;

	newImage.pixels
		= (Pixel*)malloc((newImage.width * newImage.height) * sizeof(Pixel));

	if(newImage.pixels == NULL)
		error("failed to allocate memory for newImage");

	debug("allocated memory for newImage");

	for(int i = 0; i < newImage.height; i++)
	{
		for(int j = 0; j < newImage.width; j++)
		{
			#define pixel newImage.pixels[i * newImage.width + j]
			pixel.r = 0;
			pixel.g = 0;
			pixel.b = 0;
			int count = 0;

			// take the average of all points
			for(float k = 0; k < yPixelWidth; k += yPixelWidth / SCALE)
			{
				for(float l = 0; l < xPixelWidth; l += xPixelWidth / SCALE)
				{
					#define samplePoint oldImage.pixels\
					[(int)(floor(i * yPixelWidth + k) * oldImage.width)\
					 + (int)floor(j * xPixelWidth + l)]

					pixel.r += samplePoint.r;
					pixel.g += samplePoint.g;
					pixel.b += samplePoint.b;
					count++;
				}
			}

			pixel.r = (int)((float)pixel.r / (float)count);
			pixel.g = (int)((float)pixel.g / (float)count);
			pixel.b = (int)((float)pixel.b / (float)count);
		}
	}
	return(newImage);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Video
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void playVideo(const VideoInfo INFO, const int SOUND)
{
	debug("tmp folder: %s", TMP_FOLDER);

	Image prevImage;
	prevImage.width = INFO.width;
	prevImage.height = INFO.height;

	prevImage.pixels = (Pixel*)malloc((INFO.width * INFO.height) * sizeof(Pixel));

	if(prevImage.pixels == NULL)
		error("failed to allocate memory for prevImage");

	debug("allocated memory for prevImage");

	// initialize all colors to -1 to force update when calling updateScreen()
	// for the first time
	for(int i = 0; i < INFO.height; i++)
	{
		for(int j = 0; j < INFO.width; j++)
		{
			prevImage.pixels[i * INFO.width + j].r = -1;
			prevImage.pixels[i * INFO.width + j].g = -1;
			prevImage.pixels[i * INFO.width + j].b = -1;
		}
	}

	char audioDir[1000];
	sprintf(audioDir, "%s/audio.wav", TMP_FOLDER);

	debug(
		"starting audio (%s) and video (%d fps)", audioDir, INFO.fps
	);

	if(SOUND == 1) playAudio(audioDir);

	float sTime = getTime();
	int currentFrame = 1;
	int prevFrame = 0;

	while(1)
	{
		char file[1000];
		currentFrame = (int)floor(INFO.fps * (getTime() - sTime));
		// frames start from 1
		if(currentFrame < 1)currentFrame = 1;
		sprintf(file, "%s/frame%d.bmp", TMP_FOLDER, currentFrame);

		if(currentFrame != prevFrame) // don't draw same frame twice
		{
			if(access(file, F_OK) != - 1)
			{
				Image currentImage = loadImage(file);
				updateScreen(currentImage, prevImage);
				prevImage = copyImage(currentImage);
				freeImage(&currentImage);
				// delete old frames
				remove(file);
			}
			else
			{
				debug("next file (%s) not found", file);
				freeImage(&prevImage);
				break;
			}
		}
		prevFrame = currentFrame;
	}
	freeImage(&prevImage);
}

VideoInfo getVideoInfo(const char TARGET[])
{
	VideoInfo info;

	AVFormatContext *formatCtx = avformat_alloc_context();

	if(formatCtx == NULL)
		error("failed to allocate memory for formatCtx");

	debug("allocated memory for formatCtx");

	avformat_open_input(&formatCtx, TARGET, NULL, NULL);
	avformat_find_stream_info(formatCtx,  NULL);

	int index = 0;

	for (int i = 0; i < formatCtx->nb_streams; i++)
	{
		AVCodecParameters *codecPram = formatCtx->streams[i]->codecpar;
		if (codecPram->codec_type == AVMEDIA_TYPE_VIDEO) {
			info.width = codecPram->width;
			info.height = codecPram->height;
			index = i;
			break;
		}
	}

	info.fps = formatCtx->streams[index]->r_frame_rate.num;
	info.frameCount = formatCtx->streams[index]->duration;

	debug(
		__func__,
		"got video info: %d * %d, fps = %d, frameCount = %d",
		info.width, info.height, info.fps, info.frameCount
	);

	free(formatCtx);

	return(info);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Cleanup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void cleanup()
{
	// move cursor to bottom right and reset colors
	printf("\x1b[0m \033[%d;%dH\n", getWinWidth(), getWinHeight());

	char dirName[] = TMP_FOLDER;

	debug("tmp folder: %s", dirName);

	DIR *dir = opendir(dirName);

	if(dir == NULL)
	{
		debug("failed to open tmp folder");
		exit(0);
	}

    struct dirent *next_file;
    char filepath[NAME_MAX * 2 + 1];

	int count = 0;

	// delete all images that were left
    while((next_file = readdir(dir)) != NULL)
    {
        sprintf(filepath, "%s/%s", dirName, next_file->d_name);
        remove(filepath);
		count++;
    }
    closedir(dir);

	debug("deleted %d images", count);

	stopAudio();

	exit(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Main
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//---- video ------------------------------------------------------------------//

void video(
	const int WIDTH, const int HEIGHT,
	const int FPS, const int FLAG, const char INPUT[], const int SOUND
)
{
	VideoInfo info = getVideoInfo(INPUT);

	float zoomX, zoomY;
	if(WIDTH == -1 && HEIGHT == -1)
	{
		zoomX = min(
			(float)getWinWidth() / (float)info.width,
			(float)getWinHeight() / (float)info.height
		);
		zoomY = zoomX;
	}
	else if(HEIGHT == -1)
	{
		zoomX = (float)WIDTH / (float)info.width;
		zoomY = zoomX;
	}
	else if(WIDTH == -1)
	{
		zoomY = (float)HEIGHT / (float)info.height;
		zoomX = zoomY;
	}
	else
	{
		zoomX = (float)WIDTH / (float)info.width;
		zoomY = (float)HEIGHT / (float)info.height;
	}

	debug("zoom: x: %f,  y: %f", zoomX, zoomY);

	if(FLAG == 0)
		info.fps = DEFAULT_FPS;

	if(FPS != -1)
		info.fps = FPS;

	char dir[] = TMP_FOLDER;

	debug("tmp folder: %s", dir);

	struct stat sb;

	// make /tmp/tmv folder if none exists
	if(stat(dir, &sb) != 0)
	{
		debug("could not find tmp folder");
		mkdir(dir, 0700);
		debug("created tmp folder: %s", dir);
	}

	// decode video with ffmpeg into audio
	char commandA[1000];
	sprintf(
		commandA,
		"ffmpeg -i %s -f wav %s/audio.wav >>/dev/null 2>>/dev/null",
		INPUT, dir
	);

	// decode video with ffmpeg into bmp files
	char commandB[1000];
	sprintf(
		commandB,
		"ffmpeg -i %s -vf \"fps=%d, scale=%d:%d\" %s/frame%%d.bmp >>/dev/null 2>>/dev/null",
		INPUT, info.fps, (int)(info.width * zoomX), (int)(info.height * zoomY),
		dir
	);

	clear();

	debug("forking");

	// child = plays video, parent = decodes
	int pid = fork();

	if(pid == 0)
	{

		char TARGET[1000];
		sprintf(TARGET, "%s/frame%d.bmp", dir, 1);
		// wait for first image (ffmpeg takes time to start)
		while(access(TARGET, F_OK) == -1){}
		// play the video
		playVideo(info, SOUND);
	}
	else
	{
		// audio first
		system(commandA);
		system(commandB);
		// wait for video to finish
		wait(NULL);
	}
}

//---- image ------------------------------------------------------------------//

void image(const int WIDTH, const int HEIGHT, const char INPUT[])
{
	debug("target image: %s", INPUT);

	Image image = loadImage(INPUT);

	debug(
		"original image dimensions: %d * %d",
		image.width, image.height
	);

	float zoomX, zoomY;

	if(WIDTH == -1 && HEIGHT == -1)
	{
		zoomX = min(
			(float)getWinWidth() / (float)image.width,
			(float)getWinHeight() / (float)image.height
		);
		zoomY = zoomX;
	}
	else if(HEIGHT == -1)
	{
		zoomX = (float)WIDTH / (float)image.width;
		zoomY = zoomX;
	}
	else if(WIDTH == -1)
	{
		zoomY = (float)HEIGHT / (float)image.height;
		zoomX = zoomY;
	}
	else
	{
		zoomX = (float)WIDTH / (float)image.width;
		zoomY = (float)HEIGHT / (float)image.height;
	}

	debug("zoom: x: %f, y: %f", zoomX, zoomY);

	clear();

	displayImage(scaleImage(image, zoomX, zoomY));

	freeImage(&image);
}

//---- main ------------------------------------------------------------------//

int main(int argc, char *argv[])
{
	// cleanup on ctr+c
	signal(SIGINT, cleanup);

	// setup argp
	struct args args = {0};

	// default values
	args.fps = -1;
	args.fpsFlag = 0;
	args.width = -1;
	args.height = -1;
	args.sound = 1;

	argp_parse(&argp, argc, argv, 0, 0, &args);

	debug("target file: %s", args.input);

	if(access(args.input, F_OK) == -1)
		error("%s does not exist", args.input);

	char *ext = getExtension(args.input);

	debug("file extension: %s", ext);

	int fileType = checkFileType(ext);

	if(fileType == 1)
		image(args.width, args.height, args.input);
	else if(fileType == 2)
		video(
			args.width, args.height, args.fps,
			args.fpsFlag, args.input, args.sound
		);
	else
		error("invalid file type");

	cleanup();

	return(0);
}
