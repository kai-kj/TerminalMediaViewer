#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/wait.h>
#include<signal.h>

#define FIFO_FILE "/tmp/audiofifo"

struct termios orig_termios;

float getTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long int time = (long int)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
	return((float)(time % 10000000) / 1000);
}

void reset_terminal_mode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
	struct termios new_termios;

	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	atexit(reset_terminal_mode);
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

void start(float sec)
{
	int pid = fork();

	if(pid == 0)
	{
		char command[1000];
		sprintf(command, "mplayer -ss %f audio.wav >>/dev/null 2>>/dev/null < %s", sec, FIFO_FILE);
		system(command);
		exit(0);
	}

	char command[1000];
	sprintf(command, "echo -n \"\" > %s", FIFO_FILE);

	system(command);
}

void stop()
{
	char command[1000];
	sprintf(command, "echo -n \"q\" > %s", FIFO_FILE);
	system(command);
}

int main(int argc, char *argv[])
{
	int key;

	fflush(stdout);

	set_conio_terminal_mode();

	int pause = 0;

	float startTime = getTime();

	float offset = 0;
	float pauseTime = 0;

	float pauseStart = 0;

	int playing = 0;

	while(1)
	{
		if(kbhit())
		{

			key = getch();

			if(key == 3)
				break;
			
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

		if(pause == 1 && playing == 1)
		{
			stop();
			playing = 0;
		}

		if(pause == 0 && playing == 0)
		{
			start(time);
			playing = 1;
		}
		
		system("clear");
		printf("time: %f | ", time);
		printf("start time: %f | ", startTime);
		printf("pause: %d | ", pause);
		printf("pause +  offset: %f\n", pauseTime + offset);

	}

	reset_terminal_mode();
}