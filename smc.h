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

#ifndef SMC_H
#define SMC_H

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

/* data port used by Apple SMC */
#define APPLESMC_DATA_PORT	0x300
/* command/status port used by Apple SMC */
#define APPLESMC_CMD_PORT	0x304

#define APPLESMC_MAX_DATA_LENGTH 32

/* wait up to 128 ms for a status change. */
#define APPLESMC_MIN_WAIT	0x0010
#define APPLESMC_RETRY_WAIT	0x0100
#define APPLESMC_MAX_WAIT	0x20000

#define APPLESMC_READ_CMD	0x10
#define APPLESMC_WRITE_CMD	0x11

/* Apple SMC status bits */
#define SMC_STATUS_AWAITING_DATA  0x01 /* SMC has data waiting to be read */
#define SMC_STATUS_IB_CLOSED      0x02 /* Will ignore any input */
#define SMC_STATUS_BUSY           0x04 /* Command in progress */

int read_smc(uint8_t cmd, const char *key, uint8_t *buffer, uint8_t len);
int write_smc(uint8_t cmd, const char *key, const uint8_t *buffer, uint8_t len);

#endif /* SMC_H */
