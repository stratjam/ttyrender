#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

struct frame_buffer {
	int stride;
	int w;
	int h;
	int size;
	int fd;
};

struct frame {
	struct frame_buffer* fb;
	uint8_t* data;
};

struct image {
	int w;
	int h;
	int size;
	uint8_t* data;
};

int read_range(int fd, char* buf, int len, char min, char max);

struct frame_buffer* get_frame_buffer();

void free_frame_buffer(struct frame_buffer* fb);

struct image* load_image(char* name);

void free_image(struct image* im);

struct frame* create_frame(struct frame_buffer* fb);

void free_frame(struct frame* frame);

void clear(struct frame* frame);

void draw_image(struct frame* frame, struct image* im, int fx, int fy);

void blit(struct frame* frame);

#endif
