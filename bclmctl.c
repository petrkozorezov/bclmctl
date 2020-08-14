/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2019 Evgeny Zinoviev <me@ch1p.io>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <sys/io.h>
#include <stdio.h>
#include <errno.h>
#include "smc.h"
#include "bclmctl.h"

static void print_usage(const char *name)
{
	printf("usage: %s <options>\n", name);
	printf("\n"
		"Options:\n"
		"    -h, --help:        print this help\n"
		"    -k, --key <name>:  key name\n"
		"    -t, --type <type>: data type, see below\n"
		"    --output-hex\n"
		"    --output-bin\n"
		"\n"
		"Supported data types:\n"
		"    ui8, ui16, ui32, si8, si16, flag, fpXY, spXY\n"
		"\n"
		"    fp and sp are unsigned and signed fixed point\n"
		"    data types respectively.\n"
		"\n"
		"    The X in fp and sp data types is integer bits count\n"
		"    and Y is fraction bits count.\n"
		"\n"
		"    For example,\n"
		"    fpe2 means 14 integer bits, 2 fraction bits,\n"
		"    sp78 means 7 integer bits, 8 fraction bits\n"
		"    (and one sign bit).\n"
		"\n");
}



int main(int argc, char *argv[])
{
	char* name = "BCLM";
	uint8_t len = 1;
	bool show_help = false;
	bool set_value = false;
	int opt, option_index = 0;
	long percent = 0;

	char percentbuf[PERCENT_SIZE + 1] = {0};

	struct option long_options[] = {
		{"help",       0, 0, 'h'},
		{"percent",    1, 0, 'p'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "hp:",
				  long_options, &option_index)) != EOF) {
		switch (opt) {
		case 'h':
			show_help = true;
			break;
		case 'p':
			set_value = true;
			snprintf(percentbuf, PERCENT_SIZE, "%s", optarg);
			break;
		}
	}

	if (optind < argc) {
		fprintf(stderr, "Error: Extra parameter found.\n");
		print_usage(argv[0]);
		exit(1);
	}

	percent = strtol(percentbuf, NULL, 10);

	if (set_value && (percent < 1 || percent > 100)) {
		fprintf(stderr, "percent must be between 0 and 100.");
		exit(0);
	}

	if (show_help) {
		print_usage(argv[0]);
		exit(0);
	}

	/* Check permissions */
	if (geteuid() != 0) {
		fprintf(stderr, "You must be root.\n");
		exit(1);
	}

	/* Open SMC port */
	if (ioperm(APPLESMC_DATA_PORT, 0x10, 1)) {
		fprintf(stderr, "ioperm: %s\n", strerror(errno));
		exit(1);
	}

	/* Read key from SMC */
	int retval;
	uint8_t buf;

	if (set_value) {
		buf=percent;
		retval = write_smc(APPLESMC_WRITE_CMD, name, &buf, len);
		assert(retval == 0);
	}

	buf = 0;
	retval = read_smc(APPLESMC_READ_CMD, name, &buf, len);
	assert(retval == 0);

	/* Handle returned value according to the requested type */
	printf("BCLM = %u\n", buf);

	return 0;
}
