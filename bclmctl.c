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
		"    -h, --help:              print this help\n"
		"    -p, --percent <number>:  value to write to BCLM\n"
		"\n");
}

void print_key(const char *key)
{
	int retval;
	uint8_t buf;
	retval = read_smc(APPLESMC_READ_CMD, key, &buf, 1);
	assert(retval == 0);
	printf("%s = %u\n", key, buf);
}

void set_key(const char *key, uint8_t value)
{
	int retval;
	retval = write_smc(APPLESMC_WRITE_CMD, key, &value, 1);
	assert(retval == 0);
}


int main(int argc, char *argv[])
{
	char* bclm_key = "BCLM";
	char* bfcl_key = "BFCL";
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
	if (set_value) {
		// github.com/zackelia/bclm/blob/f240f37f99757fcc0b8f38ecab0d68de26665aef/Sources/bclm/main.swift#L67
		uint8_t led_percent = percent - 5;
		set_key(bclm_key, percent);
		set_key(bfcl_key, led_percent);
	}

	print_key(bclm_key);
	print_key(bfcl_key);

	return 0;
}
