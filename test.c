#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/time.h>

struct termios orig_termios;

void reset_terminal_mode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
	struct termios new_termios;

	/* take two copies - one for now, one for later */
	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* register cleanup handler, and set the new terminal mode */
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

float getTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long int time = (long int)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
	return((float)(time % 10000000) / 1000);
}

int main(int argc, char *argv[])
{
	set_conio_terminal_mode();

	float t = 0;
	int p = 0;
	float prevTime = getTime();

	while(1)
	{
		if(kbhit())
		{
			char c = getch();
			if(c == 27)
			{
				c = getch();
				c = getch();
				switch(c)
				{
				case 67:
					t += 5;
					break;
				
				case 68:
					t -= 5;
					break;
				
				default:
					break;
				}
			}

			if(c == 32)
			{
				if(p == 0)
					p = 1;
				else if(p == 1)
					p = 0;
			}
			
			if(c == 'q')
				break;
				
		}
		if(p == 0)
			t += getTime() - prevTime;

		prevTime = getTime();

		if(t < 0)
			t = 0;
		
		system("clear");
		printf("TIME: %f[s]\n", t);
		printf("PAUSED: %d\n", p);
	}
}

/*
Arrows
	up:		65(A)
	down:	66(B)
	right:	67(C)
	left:	68(D)
Other
	space:	32
	q:		13
*/