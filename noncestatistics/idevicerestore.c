/*
 * idevicerestore.c
 * Restore device firmware and filesystem
 *
 * Copyright (c) 2010-2015 Martin Szulecki. All Rights Reserved.
 * Copyright (c) 2012-2015 Nikias Bassen. All Rights Reserved.
 * Copyright (c) 2010 Joshua Hill. All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <plist/plist.h>
#include <libgen.h>

#include "dfu.h"
#include "common.h"
#include "normal.h"
#include "recovery.h"
#include "idevicerestore.h"

struct idevicerestore_client_t* idevicerestore_client_new(void)
{
	struct idevicerestore_client_t* client = (struct idevicerestore_client_t*) malloc(sizeof(struct idevicerestore_client_t));
	if (client == NULL) {
		error("ERROR: Out of memory\n");
		return NULL;
	}
	memset(client, '\0', sizeof(struct idevicerestore_client_t));
	return client;
}

void idevicerestore_client_free(struct idevicerestore_client_t* client)
{
	if (!client) {
		return;
	}

	if (client->tss_url) {
		free(client->tss_url);
	}
	if (client->version_data) {
		plist_free(client->version_data);
	}
	if (client->nonce) {
		free(client->nonce);
	}
	if (client->udid) {
		free(client->udid);
	}
	if (client->srnm) {
		free(client->srnm);
	}
	if (client->ipsw) {
		free(client->ipsw);
	}
	if (client->version) {
		free(client->version);
	}
	if (client->build) {
		free(client->build);
	}
	if (client->restore_boot_args) {
		free(client->restore_boot_args);
	}
	if (client->cache_dir) {
		free(client->cache_dir);
	}
	free(client);
}


int check_mode(struct idevicerestore_client_t* client) {
	int mode = MODE_UNKNOWN;
	int dfumode = MODE_UNKNOWN;

	if (recovery_check_mode(client) == 0) {
		mode = MODE_RECOVERY;
	}

	else if (dfu_check_mode(client, &dfumode) == 0) {
		mode = dfumode;
	}

	else if (normal_check_mode(client) == 0) {
		mode = MODE_NORMAL;
	}

//	else if (restore_check_mode(client) == 0) {
//		mode = MODE_RESTORE;
//	}

	if (mode == MODE_UNKNOWN) {
		client->mode = NULL;
	} else {
		client->mode = &idevicerestore_modes[mode];
	}
	return mode;
}

const char* check_hardware_model(struct idevicerestore_client_t* client) {
	const char* hw_model = NULL;
	int mode = MODE_UNKNOWN;

	if (client->mode) {
		mode = client->mode->index;
	}

	switch (mode) {
//	case MODE_RESTORE:
//		hw_model = restore_check_hardware_model(client);
//		break;

	case MODE_NORMAL:
		hw_model = normal_check_hardware_model(client);
		break;

	case MODE_DFU:
	case MODE_RECOVERY:
		hw_model = dfu_check_hardware_model(client);
		break;
	default:
		break;
	}

	if (hw_model != NULL) {
		irecv_devices_get_device_by_hardware_model(hw_model, &client->device);
	}

	return hw_model;
}

int get_ecid(struct idevicerestore_client_t* client, uint64_t* ecid) {
	int mode = MODE_UNKNOWN;

	if (client->mode) {
		mode = client->mode->index;
	}

	switch (mode) {
	case MODE_NORMAL:
		if (normal_get_ecid(client, ecid) < 0) {
			*ecid = 0;
			return -1;
		}
		break;

	case MODE_DFU:
		if (dfu_get_ecid(client, ecid) < 0) {
			*ecid = 0;
			return -1;
		}
		break;

	case MODE_RECOVERY:
		if (recovery_get_ecid(client, ecid) < 0) {
			*ecid = 0;
			return -1;
		}
		break;

	default:
		error("ERROR: Device is in an invalid state\n");
		*ecid = 0;
		return -1;
	}

	return 0;
}

int get_ap_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	int mode = MODE_UNKNOWN;

	*nonce = NULL;
	*nonce_size = 0;
	if (client->mode) {
		mode = client->mode->index;
	}

	switch (mode) {
	case MODE_NORMAL:
		if (normal_get_ap_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;
	case MODE_DFU:
		if (dfu_get_ap_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;
	case MODE_RECOVERY:
		if (recovery_get_ap_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;

	default:
		info("failed\n");
		error("ERROR: Device is in an invalid state\n");
		return -1;
	}

	int i = 0;
//    info("ApNonce=");
//	for (i = 0; i < *nonce_size; i++) {
//		info("%02x", (*nonce)[i]);
//	}
//	info("\n");

	return 0;
}

int get_sep_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	int mode = MODE_UNKNOWN;

	*nonce = NULL;
	*nonce_size = 0;

	if (client->mode) {
		mode = client->mode->index;
	}

	switch (mode) {
	case MODE_NORMAL:
		if (normal_get_sep_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;
	case MODE_DFU:
		if (dfu_get_sep_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;
	case MODE_RECOVERY:
		if (recovery_get_sep_nonce(client, nonce, nonce_size) < 0) {
			info("failed\n");
			return -1;
		}
		break;

	default:
		info("failed\n");
		error("ERROR: Device is in an invalid state\n");
		return -1;
	}

	int i = 0;
    info("SepNonce=");
	for (i = 0; i < *nonce_size; i++) {
		info("%02x", (*nonce)[i]);
	}
	info("\n");

	return 0;
}
