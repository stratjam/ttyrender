#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include "graphics.h"

int read_range(int fd, char* buf, int len, char min, char max)
{
	int i;
	for (i = 0; i < len; i++) {
		if (!read(fd, buf + i, 1))
			break;

		if (buf[i] < min || buf[i] > max)
			break;
	}
	buf[i] = '\0';

	return i;
}

struct frame_buffer* get_frame_buffer()
{
	char buf[17];

	int fd = open("/sys/class/graphics/fb0/stride", O_RDONLY);
	if (fd == -1)
		return NULL;
	read_range(fd, buf, 16, '0', '9');
	int stride = atoi(buf) / 4;
	close(fd);

	fd = open("/sys/class/graphics/fb0/virtual_size", O_RDONLY);
	if (fd == -1)
		return NULL;
	read_range(fd, buf, 16, '0', '9');
	int w = atoi(buf);
	read_range(fd, buf, 16, '0', '9');
	int h = atoi(buf);
	close(fd);

	fd = open("/dev/fb0", O_RDWR);
	if (fd == -1)
		return NULL;

	struct frame_buffer* fb = malloc(sizeof(struct frame_buffer));
	if (!fb)
		return NULL;

	fb->stride = stride;
	fb->w = w;
	fb->h = h;
	fb->size = stride * h * 4;
	fb->fd = fd;

	return fb;
}

void free_frame_buffer(struct frame_buffer* fb)
{
	close(fb->fd);
	free(fb);
}

struct image* load_image(char* name)
{
	char buf[17];

	// TODO: make this .pam parser less shit
	int fd = open(name, O_RDONLY);
	if (fd == -1)
		return NULL;

	read_range(fd, buf, 16, '!', '~');

	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '0', '9');
	int w = atoi(buf);

	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '0', '9');
	int h = atoi(buf);

	int size = w * h * 4;

	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');
	read_range(fd, buf, 16, '!', '~');

	uint8_t* data = malloc(size);
	if (!data) {
		close(fd);
		return NULL;
	}
	read(fd, data, size);
	close(fd);

	for (int i = 0; i < size; i += 4) {
		uint8_t temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	struct image* im = malloc(sizeof(struct image));
	if (!im) {
		free(data);
		return NULL;
	}
	im->w = w;
	im->h = h;
	im->size = size;
	im->data = data;

	return im;
}

void free_image(struct image* im)
{
	free(im->data);
	free(im);
}

struct frame* create_frame(struct frame_buffer* fb)
{
	uint8_t* data = malloc(fb->size);
	if (!data)
		return NULL;

	struct frame* frame = malloc(sizeof(struct image));
	if (!frame) {
		free(data);
		return NULL;
	}
	frame->fb = fb;
	frame->data = data;

	return frame;
}

void free_frame(struct frame* frame)
{
	free(frame->data);
	free(frame);
}

// TODO: make this faster with asm
// trust the process
////////////////////////////////

#define CLR_PIXEL(ptr, i) \
	*(ptr + i) = 0; \
	*(ptr + i + 1) = 0; \
	*(ptr + i + 2) = 0;

#define CLR_CHUNK(ptr, i) \
	CLR_PIXEL(ptr, i); \
	CLR_PIXEL(ptr, i + 4); \
	CLR_PIXEL(ptr, i + 8); \
	CLR_PIXEL(ptr, i + 12); \
	CLR_PIXEL(ptr, i + 16); \
	CLR_PIXEL(ptr, i + 20); \
	CLR_PIXEL(ptr, i + 24); \
	CLR_PIXEL(ptr, i + 28);

void clear(struct frame* frame)
{
	register int max_i = frame->fb->size;
	register int inc = (frame->fb->stride - frame->fb->w) * 4;
	register uint8_t* ptr = frame->data;
	for (register int i = 0; i < max_i; i += inc) {
		register int inner_max = i + frame->fb->w * 4;
		while (i + 256 <= inner_max) {
			CLR_CHUNK(ptr, i);
			CLR_CHUNK(ptr, i + 32);
			CLR_CHUNK(ptr, i + 64);
			CLR_CHUNK(ptr, i + 96);
			CLR_CHUNK(ptr, i + 128);
			CLR_CHUNK(ptr, i + 160);
			CLR_CHUNK(ptr, i + 192);
			CLR_CHUNK(ptr, i + 224);
			i += 256;
		}
		while (i < inner_max) {
			CLR_PIXEL(ptr, i);
			i += 4;
		}
	}
}

#undef CLR_CHUNK
#undef CLR_PIXEL

////////////////////////////////

void draw_image(struct frame* frame, struct image* im, int fx, int fy)
{
	int min_x = MAX(0, -fx);
	int min_y = MAX(0, -fy);
	int max_x = MIN(im->w, frame->fb->w - fx);
	int max_y = MIN(im->h, frame->fb->h - fy);

	for (int y = min_y; y < max_y; y++) {
		int im_i = (y * im->w + min_x) * 4;
		int fb_i = ((y + fy) * frame->fb->stride + min_x + fx) * 4;

		for (int x = min_x; x < max_x; x++) {
			if (im->data[im_i + 3] == 255) {
				frame->data[fb_i    ] = im->data[im_i    ];
				frame->data[fb_i + 1] = im->data[im_i + 1];
				frame->data[fb_i + 2] = im->data[im_i + 2];
			}

			im_i += 4;
			fb_i += 4;
		}
	}
}

void blit(struct frame* frame)
{
	lseek(frame->fb->fd, 0, SEEK_SET);
	write(frame->fb->fd, frame->data, frame->fb->size);
}
