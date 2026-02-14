#ifndef INPUT_H_
#define INPUT_H_

#include <pthread.h>

struct input
{
	int w;
	int h;
	int x;
	int y;
	bool keys[1024];
};

struct input_conf
{
	float mult_x;
	float mult_y;
	int off_x;
	int off_y;
	int no_bounds;
};

struct input_thread
{
	struct input* in;
	struct input_conf conf;
	pthread_t thr;
	int fd;
};

struct input create_input_state(int w, int h);

void* input_thread(struct input_thread* st);

struct input_thread* start_input_thread(
		struct input* in,
		const struct input_conf conf,
		char* evfile);

void stop_input_thread(struct input_thread* st);

#endif
