#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define HELP_TEXT "keylistend v0.1.0\n" \
	"Simple daemon to listen to event devices and react to keycodes with UT app launches.\n" \
	"(C) 2021 nift4\n" \
	"\nUsage:\n" \
	"First argument: path to event device\n" \
	"All following arguments should be pairs: keycode and UT app id to launch.\n\n" \
	"Example: ./keylistend /dev/input/event3 87 dialer-app"

char err[200];
int fd = 0;

int fail(int returncode, int ignored) {
	printf("keylistend: %s\n", err);
	if (fd > 0) close(fd);
	return returncode;
}

int main(int argc, char* argv[]) {
	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf("%s\n", HELP_TEXT);
		return 0;
	}

	if ((argc-1) % 2 == 0 || argc < 4)
		return fail(1, sprintf(err, "got %i arguments, excepted multiple of 2 plus one", argc-1));

	char cmd[(argc-2) / 2][sizeof("ubuntu-app-launch ") + 4096];
	int icode[(argc-2) / 2];
	for (int i = 0; i < (argc-2) / 2; i++) {
		icode[i] = atoi(argv[2 + (i * 2)]);
		if (icode[i] < 1)
			return fail(2, sprintf(err, "excepted non-null positive number, got %s", argv[2 + i]));

		strcat(cmd[i], "ubuntu-app-launch ");
		strncat(cmd[i], argv[3 + (i * 2)], 4096);
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0)
		return fail(3, sprintf(err, "error opening device"));	

	struct input_event ev;
	unsigned int size;

	printf("keylistend: Listening on %s for codes:\n", argv[1]);
	for (int i = 0; i < (argc-2) / 2; i++)
		printf("%i\n", icode[i]);

	while (1) {
		size = read(fd, &ev, sizeof(struct input_event));
		if (size < sizeof(struct input_event))
			return fail(4, sprintf(err, "error reading from device"));
#ifdef SCANEVENTS
		printf("Event: time %ld.%06ld, ", ev.time.tv_sec, ev.time.tv_usec);
		printf("type: %i, code: %i, value: %i\n", ev.type, ev.code, ev.value);
#endif
		if (ev.value != 1)
			continue;
		for (int i = 0; i < (argc-2) / 2; i++) {
			if (ev.code == icode[i] && fork() == 0)
				system(cmd[i]);
		}
	}
}
