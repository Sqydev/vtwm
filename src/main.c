#include "./coredata.h"

#include "./utils/utils.h"
#include "./compositor/compositor.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include <wlr/util/log.h>

Coredata DATA;

static void HandleOpts(int* argc, char*** argv) {
	struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"debug", no_argument, NULL, 'd'},
	};

	char opt = 0;
	while((opt = getopt_long(*argc, *argv, "hd", long_options, NULL)) != -1) {
		switch(opt) {
			case 'h': {
				const char *help =
					"Usage:\n"
					"  vtwm [options]\n\n"

					"Options:\n"
					"  -h, --help\n"
					"      Shows this help message and exit.\n\n"

					"  -d --debug\n"
					"      Runs vtwm in debug mode\n";

				char n = write(STDOUT_FILENO, help, strlen(help));
				(void) n;
				exit(EXIT_SUCCESS);
			}
			case 'd': {
				DATA.debug = true;
				break;
			}
			default: {
				break;
			}
		}
	}
}

int main(int argc, char** argv) {
	InitPanic();

	DATA.debug = false;

	HandleOpts(&argc, &argv);

	InitCompositor();

	StartCompositor();

	FreeCompositor();

	return Finish();
}
