#include <unistd.h>

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
