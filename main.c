/*
 *  based on:
 *  ssss version 0.5  -  Copyright 2005,2006 B. Poettering
 *
 *  refactored by Christopher Hall 2017-01-18
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <termios.h>
#include <sys/mman.h>

#include <gmp.h>

#include "shamir.h"


// to save echo state
struct termios echo_orig, echo_off;

#define mpz_lshift(A, B, l) mpz_mul_2exp(A, B, l)
#define mpz_sizeinbits(A) (mpz_cmp_ui(A, 0) ? mpz_sizeinbase(A, 2) : 0)

// emergency abort and warning functions

void fatal(char *msg) {
	tcsetattr(0, TCSANOW, &echo_orig);
	fprintf(stderr, "%sFATAL: %s.\n", isatty(2) ? "\a" : "", msg);
	exit(1);
}

void warning(char *msg) {
	//if (! opt_QUIET) {
		fprintf(stderr, "%sWARNING: %s.\n", isatty(2) ? "\a" : "", msg);
		//}
}


// Prompt for a secret, generate shares for it

error_t field_print_cb(void* data, const char *buffer, size_t length, int number, int total) {
	FILE *stream = (FILE *)data;
	(void)length;
	fprintf(stream, "[%d/%d]: %s\n", number, total, buffer);
	return ERROR_OK;
}


void do_split(int opt_security, int opt_threshold, int opt_number, bool opt_diffusion, char *opt_token, bool opt_quiet, bool opt_hex) {

	if (! opt_quiet) {
		printf("Generating shares using a (%d,%d) scheme with ", opt_threshold, opt_number);
		if (opt_security) {
			printf("a %d bit", opt_security);
		} else {
			printf("dynamic");
		}
		printf(" security level.\n");

		fprintf(stderr, "Enter the secret, ");
		int deg = opt_security ? opt_security : MAXDEGREE;
		if (opt_hex) {
			fprintf(stderr, "as most %d hex digits: ", deg / 4);
		} else {
			fprintf(stderr, "at most %d ASCII characters: ", deg / 8);
		}
	}

	char buffer[MAXLINELEN];
	tcsetattr(0, TCSANOW, &echo_off);
	if (! fgets(buffer, sizeof(buffer), stdin)) {
		fatal("I/O error while reading secret");
	}
	tcsetattr(0, TCSANOW, &echo_orig);
	buffer[strcspn(buffer, "\r\n")] = '\0';

	fprintf(stderr, "\n\n");

	split(buffer, field_print_cb, stdout, opt_security, opt_threshold, opt_number, opt_diffusion, opt_token, opt_hex);
}


// Prompt for shares
const char *read_share(void *data, int number, int threshold, size_t size) {

	(void)size; // use internal static buffer

	static char buffer[MAXLINELEN];
	bool opt_quiet = *((bool *)data);

	if (! opt_quiet) {
		printf("Enter share [%d/%d]: ", number, threshold);
	}
	if (! fgets(buffer, sizeof(buffer), stdin)) {
		fatal("I/O error while reading shares");
		return NULL;
	}
	return buffer;
}


int main(int argc, char *argv[]) {
	char *name;
	int i;

	int opt_showversion = 0;
	int opt_help = 0;
	int opt_quiet = 0;
	int opt_QUIET = 0;
	int opt_hex = 0;
	int opt_diffusion = 1;
	int opt_security = 0;
	int opt_threshold = -1;
	int opt_number = -1;
	char *opt_token = NULL;

#if ! NOMLOCK
	if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
		switch(errno) {
		case ENOMEM:
			warning("couldn't get memory lock (ENOMEM, try to adjust RLIMIT_MEMLOCK!)");
			break;
		case EPERM:
			warning("couldn't get memory lock (EPERM, try UID 0!)");
			break;
		case ENOSYS:
			warning("couldn't get memory lock (ENOSYS, kernel doesn't allow page locking)");
			break;
		default:
			warning("couldn't get memory lock");
			break;
		}
	}
#endif

	if (getuid() != geteuid()) {
		seteuid(getuid());
	}

	tcgetattr(0, &echo_orig);
	echo_off = echo_orig;
	echo_off.c_lflag &= ~ECHO;

	opt_help = argc == 1;
	while((i = getopt(argc, argv, "vDhqQxs:t:n:w:")) != -1) {
		switch(i) {
		case 'v': opt_showversion = 1; break;
		case 'h': opt_help = 1; break;
		case 'q': opt_quiet = 1; break;
		case 'Q': opt_QUIET = opt_quiet = 1; break;
		case 'x': opt_hex = 1; break;
		case 's': opt_security = atoi(optarg); break;
		case 't': opt_threshold = atoi(optarg); break;
		case 'n': opt_number = atoi(optarg); break;
		case 'w': opt_token = optarg; break;
		case 'D': opt_diffusion = 0; break;
		default:
			exit(1);
		}
	}
	if (! opt_help && (argc != optind)) {
		fatal("invalid argument");
	}

	if ((name = strrchr(argv[0], '/')) == NULL) {
		name = argv[0];
	}

	if (strstr(name, "split")) {
		if (opt_help || opt_showversion) {
			puts("Split secrets using Shamir's Secret Sharing Scheme.\n"
			     "\n"
			     "ssss-split -t threshold -n shares [-w token] [-s level]"
			     " [-x] [-q] [-Q] [-D] [-v]"
				);
			if (opt_showversion)
				puts("\nVersion: " VERSION);
			exit(0);
		}

		if (opt_threshold < 2) {
			fatal("invalid parameters: invalid threshold value");
		}
		if (opt_number < opt_threshold) {
			fatal("invalid parameters: number of shares smaller than threshold");
		}
		if (opt_security && ! field_size_valid(opt_security)) {
			fatal("invalid parameters: invalid security level");
		}
		if (opt_token && (strlen(opt_token) > MAXTOKENLEN)) {
			fatal("invalid parameters: token too long");
		}
		do_split(opt_security, opt_threshold, opt_number, opt_diffusion, opt_token, opt_quiet, opt_hex);

	} else if (opt_help || opt_showversion) {
		puts("Combine shares using Shamir's Secret Sharing Scheme.\n"
		     "\n"
			     "ssss-combine -t threshold [-x] [-q] [-Q] [-D] [-v]");
		if (opt_showversion) {
			puts("\nVersion: " VERSION);
		}
		exit(0);

	} else { // combine

		if (opt_threshold < 2) {
			fatal("invalid parameters: invalid threshold value");
		}
		error_t err = combine(read_share, &opt_quiet, opt_threshold, opt_diffusion, opt_hex);
		if (ERROR_OK != err) {
			printf("error: %d\n", err);
			return 1;
		}
	}
	return 0;
}
