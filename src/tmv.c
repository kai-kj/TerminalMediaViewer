//============================================================================//
// Terminal Media Viewer
//
// View images and videos without leaving the console.
// Requires a terminal that supports truecolor and utf-8
//
// https://github.com/kal39/TerminalMediaViewer
//============================================================================//

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Changelog
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

v0.1.1 - Youtube Support

* Improved memory usage
* Cursor is now hidden during videos
* Supports spaces in video filenames
* Play videos directly from youtube
* Check if ffmpeg and YouTube exist before playing videos
* Better error and log messages

v0.1 - Initial release

* View images
* Watch videos (with sound)
* Resize images / videos

*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Usage
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

tmv [OPTION...] [INPUT FILE / URL]

  -y, --youtube              play video from youtube.
  -h, --height=[height]      Set output height.
  -w, --width=[width]        Set output width.
  -f, --fps=[target fps]     Set target fps. Default 15 fps
  -F, --origfps              Use original fps from video. Default 15 fps.
  -s, --no-sound             disable sound.
  -?, --help                 Give this help list.
	  --usage                Give a short usage message.
  -V, --version              Print program version.

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
#define _DEFAULT_SOURCE

//-------- standard libraries ------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <argp.h>
#include <termios.h>

//-------- POSIX libraries ---------------------------------------------------//

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

//-------- ffmpeg ------------------------------------------------------------//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

//-------- external libraries ------------------------------------------------//

// reading images <https://github.com/nothings/stb>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

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
	float duration;
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

Image blankImage(const int WIDTH, const int HEIGHT)
{
	Image image;
	image.width = WIDTH;
	image.height = HEIGHT;

	image.pixels = malloc((image.width * image.height) * sizeof(Pixel));

	if(image.pixels == NULL)
		error("failed to allocate memory for prevImage");

	for(int i = 0; i < image.height; i++)
	{
		for(int j = 0; j < image.width; j++)
		{
			int index = i * image.width + j;
			image.pixels[index].r = -1;
			image.pixels[index].g = -1;
			image.pixels[index].b = -1;
		}
	}

	return(image);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Debug
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

float getTime();

void logFunc(const char *FUNC, const char *FMT, ...)
{
	#ifdef DEBUG
		FILE *fp = fopen("log.txt", "a");
		char msg[4096];
		va_list args;
		va_start(args, FMT);
		vsnprintf(msg, sizeof(msg), FMT, args);
		fprintf(fp, "%f (%s): %s\n", getTime(), FUNC, msg);
		va_end(args);
		fclose(fp);
	#endif
}

#define log(a, ...) logFunc(__func__, (a), ##__VA_ARGS__)

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
	{"youtube", 'y', 0, 0, "play video from youtube\
(use url for input file)", 1},
	{"width", 'w', "[width]", 0, "Set output width.", 2},
	{"height", 'h', "[height]", 0, "Set output height.", 2},
	{"fps", 'f', "[target fps]", 0, "Set target fps. Default 15 fps", 3},
	{"origfps", 'F', 0, 0, "Use original fps from video. Default 15 fps", 3},
	{"no-sound", 's', 0, 0, "disable sound", 3},
	{"no-info", 'i', 0, 0, "disable progress bar for videos", 3},
	{ 0 }
};

struct args {
	char *input;
	int width;
	int height;
	int fps;
	int fpsFlag;
	int sound;
	int youtube;
	int bar;
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
		case 'y':
			args->youtube = 1;
			break;
		case 'i':
			args->bar = 1;
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

int max(int a, int b)
{
	if(a > b) return(a);
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

char *replaceWord(const char *STRING, const char *OLD, const char *NEW)
{
	char *result;
	int i, cnt = 0;
	int newWlen = strlen(NEW);
	int oldWlen = strlen(OLD);

	// Counting the number of times old word
	// occur in the string
	for (i = 0; STRING[i] != '\0'; i++)
	{
		if (strstr(&STRING[i], OLD) == &STRING[i])
		{
			cnt++;

			// Jumping to index after the old word.
			i += oldWlen - 1;
		}
	}

	// Making new string of enough length
	result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

	i = 0;
	while (*STRING)
	{
		// compare the substring with the result
		if (strstr(STRING, OLD) == STRING)
		{
			strcpy(&result[i], NEW);
			i += newWlen;
			STRING += oldWlen;
		}
		else
			result[i++] = *STRING++;
	}

	result[i] = '\0';
	return result;
}

int getDigits(int input)
{
	int count = 0;
	do
	{
		count++;
		input /= 10;
	} while(input != 0);
	return(count);
}

int getWinWidth();
int getWinHeight();

void getZoom(
	const int INPUT_WIDTH, const int INPUT_HEIGHT,
	const int FILE_WIDTH, const int FILE_HEIGHT,
	float *zoomX, float *zoomY
	)
{
	if(INPUT_WIDTH == -1 && INPUT_HEIGHT == -1)
	{
		*zoomX = min(
			(float)getWinWidth() / (float)FILE_WIDTH,
			(float)getWinHeight() / (float)FILE_HEIGHT
		);
		*zoomY = *zoomX;
	}
	else if(INPUT_HEIGHT == -1)
	{
		*zoomX = (float)INPUT_WIDTH / (float)FILE_WIDTH;
		*zoomY = *zoomX;
	}
	else if(INPUT_WIDTH == -1)
	{
		*zoomY = (float)INPUT_HEIGHT / (float)FILE_HEIGHT;
		*zoomX = *zoomY;
	}
	else
	{
		*zoomX = (float)INPUT_WIDTH / (float)FILE_WIDTH;
		*zoomY = (float)INPUT_HEIGHT / (float)FILE_HEIGHT;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Keyboard Input
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

struct termios orig_termios;

void resetTerminalMode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}

void setTerminalMode()
{
	struct termios new_termios;
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Cleanup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void cleanup()
{
	resetTerminalMode();

	// move cursor to bottom right and reset colors and show cursor
	printf("\x1b[0m\033[?25h\033[%d;%dH\n", getWinWidth(), getWinHeight());
	char dirName[] = TMP_FOLDER;

	log("tmp folder: %s", dirName);

	DIR *dir = opendir(dirName);

	if(dir == NULL)
	{
		log("failed to open tmp folder");
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

	log("deleted %d images", count);

	log("terminating program");
	
	exit(0);
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

// only updates changed pixels
void updateScreen(Image image, Image prevImage)
{
	//Hide cursor (avoids that one white pixel when playing video)
	printf("\033[?25l");

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

	//log("allocated mempry for image");

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

	log("allocated memory for newImage");

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

void initImage(const int WIDTH, const int HEIGHT, const char INPUT[])
{
	log("target image: %s", INPUT);

	Image image = loadImage(INPUT);

	log("original image dimensions: %d * %d", image.width, image.height);

	float zoomX, zoomY;

	getZoom(WIDTH, HEIGHT, image.width, image.height, &zoomX, &zoomY);

	log("zoom: x: %f, y: %f", zoomX, zoomY);

	image = scaleImage(image, zoomX, zoomY);

	Image prevImage = blankImage(image.width, image.height);

	clear();

	updateScreen(image, prevImage);

	freeImage(&image);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Video
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int displayFrame(int frame)
{
	char fileName[1000];
	sprintf(fileName, "%s/frame%d.bmp", TMP_FOLDER, frame);

	if(access(fileName, F_OK) != - 1)
	{
		Image currentFrame, previousFrame;
		currentFrame = loadImage(fileName);

		if(frame > 2)
		{
			sprintf(fileName, "%s/frame%d.bmp", TMP_FOLDER, frame - 1);
			previousFrame = loadImage(fileName);
		}
		else
			previousFrame = blankImage(currentFrame.width, currentFrame.height);
		
		updateScreen(currentFrame, previousFrame);

		freeImage(&currentFrame);
		freeImage(&previousFrame);
	}
	else
	{
		log("next file (%s) not found", fileName);
		return(1);
	}

	return(0);
}

void videoUI(const VideoInfo INFO, const float TIME)
{
	//move cursor to bottom left
	printf("\033[%d;%dH", getWinHeight(), 0);

	// reset colors
	printf("\e[40m\e[97m");

	//print time
	printf(
		"%02d:%02d / %02d:%02d ",
		(int)(TIME / 60),
		(int)TIME % 60,
		(int)(INFO.duration / 60),
		(int)INFO.duration % 60
	);

	int offset = max(2, getDigits((int)(TIME / 60)))
		+ max(2, getDigits((int)(INFO.duration / 60))) + 11;

	int lineLength
		= (int)((float)(INFO.width - offset) * (TIME / INFO.duration));

	// print red bar
	printf("\e[31m");
	for(int i = 0; i < lineLength; i++)
		printf("▬");

	// print gray bar
	printf("\e[90m");
	for(int i = 0; i < INFO.width - offset - lineLength; i++)
		printf("▬");
}

void videoLoop(const VideoInfo INFO, const int SOUND, const int BAR)
{
	log("tmp folder: %s", TMP_FOLDER);

	char audioDir[1000];
	sprintf(audioDir, "%s/audio.wav", TMP_FOLDER);

	log(
		"starting audio (%s) and video (%d fps)", audioDir, INFO.fps
	);

	setTerminalMode();

	clear();

	int pause = 0;

	float startTime = getTime();

	float offset = 0;
	float pauseTime = 0;

	float pauseStart = 0;

	// main loop
	while(1)
	{
		if(kbhit())
		{
			char key = getch();

			if(key == 3)
				cleanup();
			
			else if(key == 27)
			{
				getch();
				key = getch();
					
				switch(key)
				{
					case 67:
						offset -= 5;
						break;
					case 68:
						offset += 5;
						break;
				}
			}
			else if(key == ' ')
			{
				if(pause == 1)
					pause = 0;
				else if( pause == 0)
				{
					pause = 1;
					pauseStart = getTime();
				}
			}
		}

		if(pause == 1)
			pauseTime = getTime() - pauseStart;

		float time = getTime() - startTime - offset - pauseTime;

		if(time < 0)
		{
			offset += getTime() - startTime - offset - pauseTime;
			time = getTime() - startTime - offset - pauseTime;
		}

		int frame = (int)floor(INFO.fps * time);

		if(frame < 1)
			frame = 1;

		int check = displayFrame(frame);

		if(check == 1)
		{
			log("exiting...");
			cleanup();
		}
		
		if(BAR == 0)
			videoUI(INFO ,time);
	}
}

VideoInfo getVideoInfo(const char TARGET[])
{
	av_register_all();

	VideoInfo info;

	AVFormatContext *formatCtx = avformat_alloc_context();

	if(formatCtx == NULL)
		error("failed to allocate memory for formatCtx");

	log("allocated memory for formatCtx");

	if(avformat_open_input(&formatCtx, TARGET, NULL, NULL) < 0)
		error("failed to open file");

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
	info.duration = formatCtx->duration / AV_TIME_BASE;

	log(
		"got video info: %d * %d, fps = %d, time = %f[min]",
		info.width, info.height, info.fps,
		info.duration / 60
	);

	free(formatCtx);

	return(info);
}

void decodeVideo(const char INPUT[], const VideoInfo VIDINFO)
{
	struct stat sb;

	// make /tmp/tmv folder if none exists
	if(stat(TMP_FOLDER, &sb) != 0)
	{
		log("could not find tmp folder");
		mkdir(TMP_FOLDER, 0700);
		log("created tmp folder: %s", TMP_FOLDER);
	}

	char commandA[1000], commandB[1000];

	// decode video with ffmpeg into audio
	sprintf(
		commandA,
		"ffmpeg -i \"%s\" -f wav \"%s/audio.wav\" >>/dev/null 2>>/dev/null",
		INPUT, TMP_FOLDER
	);

	// decode video with ffmpeg into bmp files
	sprintf(
		commandB,
		"ffmpeg -i \"%s\" -vf \"fps=%d, scale=%d:%d\" \"%s/frame%%d.bmp\"\
 >>/dev/null 2>>/dev/null",
		INPUT, VIDINFO.fps, (int)(VIDINFO.width), (int)(VIDINFO.height),
		TMP_FOLDER
	);

	log("audio command: %s", commandA);
	log("video command: %s", commandB);

	system(commandA);
	system(commandB);
}

void initVideo(
	const int WIDTH, const int HEIGHT,
	const int FPS, const int FLAG, const char INPUT[],
	const int SOUND, const int BAR
)
{
	log("target: %s", INPUT);

	//check if ffmpeg is installed
	if(system("ffmpeg -h >>/dev/null 2>>/dev/null") != 0)
		error("ffmpeg is not installed");

	VideoInfo vidInfo = getVideoInfo(INPUT);

	float zoomX, zoomY;

	getZoom(WIDTH, HEIGHT, vidInfo.width, vidInfo.height, &zoomX, &zoomY);

	log("zoom: x: %f, y: %f", zoomX, zoomY);	

	vidInfo.width *= zoomX;
	vidInfo.height *= zoomY;

	if(FLAG == 0)
		vidInfo.fps = DEFAULT_FPS;

	if(FPS != -1)
		vidInfo.fps = FPS;

	log("forking");

	// child = plays video, parent = decodes
	int pid = fork();

	if(pid == 0)
	{
		char TARGET[1000];
		sprintf(TARGET, "%s/frame1.bmp", TMP_FOLDER);
		// wait for first image (ffmpeg takes time to start)
		while(access(TARGET, F_OK) == -1){}
		// play the video
		videoLoop(vidInfo, SOUND, BAR);
	}
	else
	{
		decodeVideo(INPUT, vidInfo);
		wait(NULL);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Youtube
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void initYoutube(
	const int WIDTH, const int HEIGHT,
	const int FPS, const int FLAG, const char INPUT[],
	const int SOUND, const int BAR
)
{
	//check if youtube-dl is installed
	if(system("youtube-dl -h >>/dev/null 2>>/dev/null") != 0)
		error("youtube-dl is not installed");

	struct stat sb;

	// make /tmp/tmv folder if none exists
	if(stat(TMP_FOLDER, &sb) != 0)
	{
		log("could not find tmp folder");
		mkdir(TMP_FOLDER, 0700);
		log("created tmp folder: %s", TMP_FOLDER);
	}

	// download video with youtube-dl
	char command[1000];
	sprintf(
		command,
		"youtube-dl --geo-bypass --ignore-config -q --no-warnings -f mp4 \
-o \"%s/video.%%(ext)s\" %s >>/dev/null 2>>/dev/null",
		TMP_FOLDER, INPUT
	);

	log("command: %s", command);
	
	if(system(command) != 0)
		error("could not download video");

	log("finished video download");

	char dir[1000];

	sprintf(dir, "%s/video.mp4", TMP_FOLDER);

	initVideo(WIDTH, HEIGHT, FPS, FLAG, dir, SOUND, BAR);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Main
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int main(int argc, char *argv[])
{
	log("\n\n---- NEW PROCESS ----\n");

	// cleanup on ctr+c
	signal(SIGINT, cleanup);

	// save terminal config
	tcgetattr(0, &orig_termios);

	// setup argp
	struct args args = {0};

	// default values
	args.fps = -1;
	args.fpsFlag = 0;
	args.width = -1;
	args.height = -1;
	args.sound = 1;
	args.youtube = 0;
	args.bar = 0;

	// parse arguments
	argp_parse(&argp, argc, argv, 0, 0, &args);

	// check file type
	if(args.youtube == 1)
	{
		// youtube
		log("youtube mode");
		initYoutube(
			args.width, args.height, args.fps,
			args.fpsFlag, args.input,
			args.sound, args.bar
		);
	}
	else
	{
		log("target file: %s", args.input);

		if(access(args.input, F_OK) == -1)
			error("%s does not exist. If it is a url use the -y flag", args.input);

		char *ext = getExtension(args.input);

		log("file extension: %s", ext);

		int fileType = checkFileType(ext);

		// image
		if(fileType == 1)
			initImage(args.width, args.height, args.input);

		// video
		else if(fileType == 2)
			initVideo(
				args.width, args.height, args.fps,
				args.fpsFlag, args.input,
				args.sound, args.bar
			);

		// other
		else
			error("invalid file type");
	}

	cleanup();

	return(0);
}
