#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG 0

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

int main(int argc, char *argv[])
{
	char PATH[100];
	sprintf(PATH, "%s", argv[1]);

	int winX = getWinX();
	int winY = getWinY();

	if(DEBUG) printf("DEBUG: got window size (%d, %d)\n", winX, winY);

	if(winX <= 0 || winY <= 0)
	{
		printf("ERROR: invalid window size\n");
		exit(1);
	}

	Image image = loadImage(PATH);

	if(DEBUG) printf(
		"DEBUG: got unscaled image (%d * %d)\n", image.width, image.height
	);

	float zoom = min(
		(float)winX / (float)image.width, (float)winY / (float)image.height
	);

	if(DEBUG) printf("DEBUG: zoom = %f\n", zoom);

	image = scaleImage(image, zoom);

	if(DEBUG) printf(
		"DEBUG: got scaled image (%d * %d)\n", image.width, image.height
	);

	if(DEBUG) printf("DEBUG: displaying image...\n");

	displayImage(image);

	return(0);
}
