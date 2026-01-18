#ifndef INPUT_H_
#define INPUT_H_

struct input {
	int w;
	int h;
	int x;
	int y;
	bool keys[1024];
};

struct input_conf {
	float abs_mult_x;
	float abs_mult_y;
	float abs_off_x;
	float abs_off_y;
	float rel_mult_x;
	float rel_mult_y;
};

struct input_thread {
	struct input* in;
	struct input_conf conf;
	int abs_w_min;
	int abs_w_max;
	int abs_h_min;
	int abs_h_max;
	pthread_t thr;
	int fd;
};

int map(int n, int s1, int s2, int d1, int d2);

struct input create_input_state(int w, int h);

void* input_thread(struct input_thread* st);

int start_input_thread(
		struct input* in,
		const struct input_conf conf,
		char* evfile,
		struct input_thread** st_ptr);

void stop_input_thread(struct input_thread* st);

#endif
