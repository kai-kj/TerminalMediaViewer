#include <argp.h>
#include <stdlib.h>

char doc[] = "A program which accepts an input and output file as arguments";
char args_doc[] = "INPUT";

static struct argp_option options[] = {
    {"fps", 'F', "FPS", 0, "Set fps" },
    {"width", 'w', "WIDTH", 0, "Set width"},
	{"height", 'h', "HEIGHT", 0, "Set height"},
	{ 0 }
};

struct arguments {
	char *input;
	int fps;
	int width;
	int height;
};

static error_t parse_option(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch(key)
	{
		case ARGP_KEY_ARG:
			if(state->arg_num == 0)
				arguments->input = arg;
			else
				argp_usage( state );

			break;
		case 'F':
			arguments->fps = atoi(arg);
			break;
		case 'w':
			arguments->width = atoi(arg);
			break;
		case 'h':
			arguments->height = atoi(arg);
			break;
		case ARGP_KEY_END:
			if (arguments->input == NULL)
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

int main( int argc, char *argv[] )
{
	struct arguments arguments = {0};

	arguments.fps = -1;
	arguments.width = -1;
	arguments.height = -1;

	argp_parse( &argp, argc, argv, 0, 0, &arguments );

	printf("INPUT: %s\n", arguments.input);
	printf("fps = %d\n", arguments.fps);
	printf("width = %d\n", arguments.width);
	printf("height = %d\n", arguments.height);
	return 0;
}
