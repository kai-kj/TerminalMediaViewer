//============================================================================//
// Terminal Media Viewer
//============================================================================//

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

//-------- POSIX libraries ---------------------------------------------------//

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

//-------- ffmpeg ------------------------------------------------------------//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

//-------- external libraries ------------------------------------------------//

// reading images <https://github.com/nothings/stb>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Types
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

char vFormats[][25] = {"3dostr", "3g2", "3gp", "4xm", "a64", "aa", "aac", "ac3", "acm", "act", "adf", "adp", "ads", "adts", "adx", "aea", "afc", "aiff", "aix", "alaw", "alias_pix", "alsa", "amr", "amrnb", "amrwb", "anm", "apc", "ape", "apng", "aptx", "aptx_hd", "aqtitle", "asf", "asf_o", "asf_stream", "ass", "ast", "au", "avi", "avm2", "avr", "avs", "avs2", "bethsoftvid", "bfi", "bfstm", "bin", "bink", "bit", "bmp_pipe", "bmv", "boa", "brender_pix", "brstm", "c93", "caf", "cavsvideo", "cdg", "cdxl", "cine", "codec2", "codec2raw", "concat", "crc", "dash", "data", "daud", "dcstr", "dds_pipe", "dfa", "dhav", "dirac", "dnxhd", "dpx_pipe", "dsf", "dsicin", "dss", "dts", "dtshd", "dv", "dvbsub", "dvbtxt", "dvd", "dxa", "ea", "ea_cdata", "eac3", "epaf", "exr_pipe", "f32be", "f32le", "f4v", "f64be", "f64le", "fbdev", "ffmetadata", "fifo", "fifo_test", "film_cpk", "filmstrip", "fits", "flac", "flic", "flv", "framecrc", "framehash", "framemd5", "frm", "fsb", "g722", "g723_1", "g726", "g726le", "g729", "gdv", "genh", "gif", "gif_pipe", "gsm", "gxf", "h261", "h263", "h264", "hash", "hcom", "hds", "hevc", "hls", "hnm", "ico", "idcin", "idf", "iec61883", "iff", "ifv", "ilbc", "image2", "image2pipe", "ingenient", "ipmovie", "ipod", "ircam", "ismv", "iss", "iv8", "ivf", "ivr", "j2k_pipe", "jack", "jacosub", "jpeg_pipe", "jpegls_pipe", "jv", "kmsgrab", "kux", "latm", "lavfi", "libmodplug", "live_flv", "lmlm4", "loas", "lrc", "lvf", "lxf", "m4v", "matroska", "matroska", "md5", "mgsts", "microdvd", "mjpeg", "mjpeg_2000", "mkvtimestamp_v2", "mlp", "mlv", "mm", "mmf", "mov", "mov", "mp2", "mp3", "mp4", "mpc", "mpc8", "mpeg", "mpeg1video", "mpeg2video", "mpegts", "mpegtsraw", "mpegvideo", "mpjpeg", "mpl2", "mpsub", "msf", "msnwctcp", "mtaf", "mtv", "mulaw", "musx", "mv", "mvi", "mxf", "mxf_d10", "mxf_opatom", "mxg", "nc", "nistsphere", "nsp", "nsv", "null", "nut", "nuv", "oga", "ogg", "ogv", "oma", "opus", "oss", "paf", "pam_pipe", "pbm_pipe", "pcx_pipe", "pgm_pipe", "pgmyuv_pipe", "pictor_pipe", "pjs", "pmp", "png_pipe", "ppm_pipe", "psd_pipe", "psp", "psxstr", "pulse", "pva", "pvf", "qcp", "qdraw_pipe", "r3d", "rawvideo", "realtext", "redspark", "rl2", "rm", "roq", "rpl", "rsd", "rso", "rtp", "rtp_mpegts", "rtsp", "s16be", "s16le", "s24be", "s24le", "s32be", "s32le", "s337m", "s8", "sami", "sap", "sbc", "sbg", "scc", "sdl", "sdp", "sdr2", "sds", "sdx", "segment", "ser", "sgi_pipe", "shn", "siff", "singlejpeg", "sln", "smjpeg", "smk", "smoothstreaming", "smush", "sol", "sox", "spdif", "spx", "srt", "stl", "stream_segment", "subviewer", "subviewer1", "sunrast_pipe", "sup", "svag", "svcd", "svg_pipe", "swf", "tak", "tedcaptions", "tee", "thp", "tiertexseq", "tiff_pipe", "tmv", "truehd", "tta", "tty", "txd", "ty", "u16be", "u16le", "u24be", "u24le", "u32be", "u32le", "u8", "uncodedframecrc", "v210", "v210x", "vag", "vc1", "vc1test", "vcd", "vidc", "video4linux2", "vividas", "vivo", "vmd", "vob", "vobsub", "voc", "vpk", "vplayer", "vqf", "w64", "wav", "wc3movie", "webm", "webm_chunk", "webm_dash_manifest", "webp", "webp_pipe", "webvtt", "wsaud", "wsd", "wsvqa", "wtv", "wv", "wve", "x11grab", "xa", "xbin", "xmv", "xpm_pipe", "xv", "xvag", "xwd_pipe", "xwma", "yop", "yuv4mpegpip", "END"};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Types
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Debug
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int debug(const char *fmt, ...)
{
	#ifdef DEBUG
	char msg[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
	printf("DEBUG: %s\n", msg);
    va_end(args);
    return 0;
	#else
	return 1;
	#endif
}

void error(const char *fmt, ...)
{
	char msg[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
	printf("ERROR: %s\n", msg);
    va_end(args);
	exit(1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Misc
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

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

char *getExtension(const char *PATH) {
    char *dot = strrchr(PATH, '.');
    if(!dot || dot == PATH) return "";
    return dot + 1;
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Screen
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

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
			}
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Image
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int getImageWidth(const char PATH[])
{
	int width, height, components;
	stbi_load(PATH, &width, &height, &components, 3);
	return(width);
}

int getImageHeight(const char PATH[])
{
	int width, height, components;
	stbi_load(PATH, &width, &height, &components, 3);
	return(height);
}

Image loadImage(const char PATH[])
{
	int width, height, components;

	unsigned char *imageRaw = stbi_load(PATH, &width, &height, &components, 3);

	if(imageRaw == NULL)
		error("invalid image %s", PATH);

	Image image;
	image.height = height;
	image.width = width;
	image.pixels = (Pixel*)malloc((width * height) * sizeof(Pixel));

	if(image.pixels == NULL)
		error("failed to allocate memory for image");

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

	free(imageRaw);

	return(image);
}

Image scaleImage(Image oldImage, float zoom)
{
	Image newImage;
	newImage.width = (int)(oldImage.width * zoom);
	newImage.height = (int)(oldImage.height * zoom);
	float pixelWidth = 1 / zoom;

	newImage.pixels
		= (Pixel*)malloc((newImage.width * newImage.height) * sizeof(Pixel));

	for(int i = 0; i < newImage.height; i++)
	{
		for(int j = 0; j < newImage.width; j++)
		{
			#define pixel newImage.pixels[i * newImage.width + j]
			pixel.r = 0;
			pixel.g = 0;
			pixel.b = 0;
			int count = 0;

			for(float k = 0; k < pixelWidth; k += 0.1)
			{
				#define samplePoint oldImage.pixels\
				[(int)(floor(i * pixelWidth + k) * oldImage.width)\
				 + (int)floor(j * pixelWidth + k)]

				pixel.r += samplePoint.r;
				pixel.g += samplePoint.g;
				pixel.b += samplePoint.b;
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
	return(newImage);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Video
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void playVideo(const VideoInfo INFO)
{
	char dir[100];
	sprintf(dir, "/home/%s/.tiv", getenv("USER"));

	debug("playVideo: image dir %s", dir);

	Image prevImage;
	prevImage.pixels = (Pixel*)malloc((INFO.width * INFO.height) * sizeof(Pixel));

	if(prevImage.pixels == NULL)
		error("failed to allocate memory for prevImage");

	debug("playVideo: allocated memory for prevImage");

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
	float t = time;

	debug("playVideo: start time %f[s], frames %d", time, INFO.frameCount);

	for(int i = 0; i < INFO.frameCount - 1; i++)
	{
		char path[1000];
		sprintf(path, "%s/frame%d.bmp", dir, i + 1);
		if(access(path, F_OK) != -1 )
		{
			Image currentImage = loadImage(path);
			remove(path);

			while(getTime() - time < 1 / (float)INFO.fps){}

			time = getTime();
			updateScreen(currentImage, prevImage);
			prevImage = currentImage;
		}
		else
		{
			break;
		}
	}

	debug("playVideo: end time %f[s], duration %f", getTime(), getTime() - t);
}

VideoInfo getVideoInfo(const char PATH[])
{
	VideoInfo info;

	AVFormatContext *formatCtx = avformat_alloc_context();

	if(formatCtx == NULL)
		error("failed to allocate memory for formatCtx");

	debug("getVideoInfo: allocated memory for formatCtx");

	avformat_open_input(&formatCtx, PATH, NULL, NULL);
	avformat_find_stream_info(formatCtx,  NULL);

	int index;

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
	info.duration = formatCtx->duration / 1000000;
	info.frameCount = formatCtx->streams[index]->duration;

	debug(
		"getVideoInfo: got video info {(%d * %d), fps = %d, duration = %f, frameCount = %d}",
		info.width, info.height, info.fps, info.duration, info.frameCount
	);

	free(formatCtx);

	return(info);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Cleanup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void cleanup()
{
	printf("\x1b[0m \033[%d;%dH \n", getWinWidth(), getWinHeight());

	char dirName[NAME_MAX];
	sprintf(dirName, "/home/%s/.tiv", getenv("USER"));

	debug("cleanup: image dir [%s]", dirName);

	DIR *dir = opendir(dirName);

	if(dir == NULL)
		error("failed to open dir");

    struct dirent *next_file;
    char filepath[NAME_MAX * 2 + 1];

	int count = 0;

    while((next_file = readdir(dir)) != NULL)
    {
        sprintf(filepath, "%s/%s", dirName, next_file->d_name);
        remove(filepath);
		count++;
    }
    closedir(dir);

	debug("cleanup: deleted images(%d)", count);

	exit(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Main
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int main(const int argc, const char *argv[])
{
	signal(SIGINT, cleanup);

	char PATH[100];
	sprintf(PATH, "%s", argv[1]);

	debug("MAIN: target %s", PATH);

	// 1: image, 2: video
	int fileType = 1;

	char *ext = getExtension(PATH);

	debug("MAIN: got extension %s", ext);

	int i = 0;

	while(vFormats[i])
	{
		if(strcmp(ext, vFormats[i]) == 0)
		{
			fileType = 2;
			break;
		}
		else if(strcmp("END", vFormats[i]) == 0)
		{
			break;
		}
		i++;
	}

	debug("%d", fileType);

	if(fileType == 1)
	{
		// image

		debug("MAIN: image");

		Image image = loadImage(PATH);

		float zoom = min(
			(float)getWinWidth() / image.width,
			(float)getWinHeight() / image.height
		);

		debug("MAIN: got zoom %f", zoom);

		image = scaleImage(image, zoom);

		displayImage(image);

	}
	else
	{
		// video

		debug("MAIN: video");

		VideoInfo info = getVideoInfo(PATH);

		float zoom = min(
			(float)getWinWidth() / (float)info.width,
			(float)getWinHeight() / (float)info.height
		);

		debug("MAIN: got zoom %f", zoom);

		char dir[100];
		sprintf(dir, "/home/%s/.tiv", getenv("USER"));

		debug("MAIN: image dir %s", dir);

		struct stat sb;
		if(stat(dir, &sb) != 0)
		{
			mkdir(dir, 0700);
			debug("MAIN: no image dir, created");
		}

		char command[1000];
		sprintf(
			command,
			"ffmpeg -i %s -vf \"fps=%d, scale=%d:-1\" %s/frame%%d.bmp >>/dev/null 2>>/dev/null",
			PATH, info.fps, (int)(info.width * zoom), dir
		);

		debug("MAIN: decoded video");

		system(command);

		playVideo(info);
	}

	debug("MAIN: cleanup");

	cleanup();

	return(0);
}
