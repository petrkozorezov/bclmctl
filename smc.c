/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2019 Evgeny Zinoviev <me@ch1p.io>
 *
 * SMC interacting code is based on applesmc linux driver by:
 * Copyright (C) 2007 Nicolas Boichat <nicolas@boichat.ch>
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
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

#include <stdlib.h>
#include <sys/io.h>
#include <errno.h>
#include <stdio.h>
#include "smc.h"

static int wait_status(uint8_t val, uint8_t mask)
{
	uint8_t status;
	int us;
	int i;

	us = APPLESMC_MIN_WAIT;
	for (i = 0; i < 24 ; i++) {
		status = inb(APPLESMC_CMD_PORT);
		if ((status & mask) == val)
			return 0;
		usleep(us);
		if (i > 9)
			us <<= 1;
	}
	return -EIO;
}

static int send_byte(uint8_t cmd, uint16_t port)
{
	int status;

	status = wait_status(0, SMC_STATUS_IB_CLOSED);
	if (status)
		return status;
	/*
	 * This needs to be a separate read looking for bit 0x04
	 * after bit 0x02 falls. If consolidated with the wait above
	 * this extra read may not happen if status returns both
	 * simultaneously and this would appear to be required.
	 */
	status = wait_status(SMC_STATUS_BUSY, SMC_STATUS_BUSY);
	if (status)
		return status;

	outb(cmd, port);
	return 0;
}

static int send_command(uint8_t cmd)
{
	int ret;

	ret = wait_status(0, SMC_STATUS_IB_CLOSED);
	if (ret)
		return ret;
	outb(cmd, APPLESMC_CMD_PORT);
	return 0;
}

static int smc_sane(void)
{
	int ret;

	ret = wait_status(0, SMC_STATUS_BUSY);
	if (!ret)
		return ret;
	ret = send_command(APPLESMC_READ_CMD);
	if (ret)
		return ret;
	return wait_status(0, SMC_STATUS_BUSY);
}

static int send_argument(const char *key)
{
	int i;

	for (i = 0; i < 4; i++)
		if (send_byte(key[i], APPLESMC_DATA_PORT))
			return -EIO;
	return 0;
}

int read_smc(uint8_t cmd, const char *key, uint8_t *buffer, uint8_t len)
{
	uint8_t status, data = 0;
	int i;
	int ret;

	ret = smc_sane();
	if (ret)
		return ret;

	if (send_command(cmd) || send_argument(key)) {
		fprintf(stderr, "%.4s: read arg fail\n", key);
		return -EIO;
	}

	/* This has no effect on newer (2012) SMCs */
	if (send_byte(len, APPLESMC_DATA_PORT)) {
		fprintf(stderr, "%.4s: read len fail\n", key);
		return -EIO;
	}

	for (i = 0; i < len; i++) {
		if (wait_status(SMC_STATUS_AWAITING_DATA | SMC_STATUS_BUSY,
				SMC_STATUS_AWAITING_DATA | SMC_STATUS_BUSY)) {
			fprintf(stderr, "%.4s: read data[%d] fail\n", key, i);
			return -EIO;
		}
		buffer[i] = inb(APPLESMC_DATA_PORT);
	}

	/* Read the data port until bit0 is cleared */
	for (i = 0; i < 16; i++) {
		usleep(APPLESMC_MIN_WAIT);
		status = inb(APPLESMC_CMD_PORT);
		if (!(status & SMC_STATUS_AWAITING_DATA))
			break;
		data = inb(APPLESMC_DATA_PORT);
	}
	if (i)
		fprintf(stderr, "flushed %d bytes, last value is: %d\n", i, data);

	return wait_status(0, SMC_STATUS_BUSY);
}

int write_smc(uint8_t cmd, const char *key, const uint8_t *buffer, uint8_t len)
{
	int i;
	int ret;

	ret = smc_sane();
	if (ret)
		return ret;

	if (send_command(cmd) || send_argument(key)) {
		fprintf(stderr, "%s: write arg fail\n", key);
		return -EIO;
	}

	if (send_byte(len, APPLESMC_DATA_PORT)) {
		fprintf(stderr, "%.4s: write len fail\n", key);
		return -EIO;
	}

	for (i = 0; i < len; i++) {
		if (send_byte(buffer[i], APPLESMC_DATA_PORT)) {
			fprintf(stderr, "%s: write data fail\n", key);
			return -EIO;
		}
	}

	return wait_status(0, SMC_STATUS_BUSY);
}
