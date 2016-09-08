/*
 * dfu.c
 * Functions for handling idevices in DFU mode
 *
 * Copyright (c) 2010-2013 Martin Szulecki. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libirecovery.h"

#include "dfu.h"
#include "recovery.h"
#include "idevicerestore.h"
#include "common.h"

int dfu_progress_callback(irecv_client_t client, const irecv_event_t* event) {
//	if (event->type == IRECV_PROGRESS) {
//		print_progress_bar(event->progress);
//	}
	return 0;
}

int dfu_client_new(struct idevicerestore_client_t* client) {
	int i = 0;
	int attempts = 10;
	irecv_client_t dfu = NULL;
	irecv_error_t dfu_error = IRECV_E_UNKNOWN_ERROR;

	if (client->dfu == NULL) {
		client->dfu = (struct dfu_client_t*)malloc(sizeof(struct dfu_client_t));
		memset(client->dfu, 0, sizeof(struct dfu_client_t));
		if (client->dfu == NULL) {
			error("ERROR: Out of memory\n");
			return -1;
		}
	}

	for (i = 1; i <= attempts; i++) {
		dfu_error = irecv_open_with_ecid(&dfu, client->ecid);
		if (dfu_error == IRECV_E_SUCCESS) {
			break;
		}

		if (i >= attempts) {
			error("ERROR: Unable to connect to device in DFU mode\n");
			return -1;
		}

		sleep(1);
		debug("Retrying connection...\n");
	}

	irecv_event_subscribe(dfu, IRECV_PROGRESS, &dfu_progress_callback, NULL);
	client->dfu->client = dfu;
	return 0;
}

void dfu_client_free(struct idevicerestore_client_t* client) {
	if(client != NULL) {
		if (client->dfu != NULL) {
			if(client->dfu->client != NULL) {
				irecv_close(client->dfu->client);
				client->dfu->client = NULL;
			}
			free(client->dfu);
		}
		client->dfu = NULL;
	}
}

int dfu_check_mode(struct idevicerestore_client_t* client, int* mode) {
	irecv_client_t dfu = NULL;
	irecv_error_t dfu_error = IRECV_E_SUCCESS;
	int probe_mode = -1;

	irecv_init();
	dfu_error = irecv_open_with_ecid(&dfu, client->ecid);
	if (dfu_error != IRECV_E_SUCCESS) {
		return -1;
	}

	irecv_get_mode(dfu, &probe_mode);

	if ((probe_mode != IRECV_K_DFU_MODE) && (probe_mode != IRECV_K_WTF_MODE)) {
		irecv_close(dfu);
		return -1;
	}

	*mode = (probe_mode == IRECV_K_WTF_MODE) ? MODE_WTF : MODE_DFU;

	irecv_close(dfu);

	return 0;
}

const char* dfu_check_hardware_model(struct idevicerestore_client_t* client) {
	irecv_client_t dfu = NULL;
	irecv_error_t dfu_error = IRECV_E_SUCCESS;
	irecv_device_t device = NULL;

	irecv_init();
	dfu_error = irecv_open_with_ecid(&dfu, client->ecid);
	if (dfu_error != IRECV_E_SUCCESS) {
		return NULL;
	}

	dfu_error = irecv_devices_get_device_by_client(dfu, &device);
	if (dfu_error != IRECV_E_SUCCESS) {
		return NULL;
	}

	irecv_close(dfu);

	return device->hardware_model;
}

int dfu_send_buffer(struct idevicerestore_client_t* client, unsigned char* buffer, unsigned int size)
{
	irecv_error_t err = 0;

	info("Sending data (%d bytes)...\n", size);

	err = irecv_send_buffer(client->dfu->client, buffer, size, 1);
	if (err != IRECV_E_SUCCESS) {
		error("ERROR: Unable to send data: %s\n", irecv_strerror(err));
		return -1;
	}

	return 0;
}



int dfu_get_cpid(struct idevicerestore_client_t* client, unsigned int* cpid) {
	if(client->dfu == NULL) {
		if (dfu_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->dfu->client);
	if (!device_info) {
		return -1;
	}

	*cpid = device_info->cpid;

	return 0;
}

int dfu_get_ecid(struct idevicerestore_client_t* client, uint64_t* ecid) {
	if(client->dfu == NULL) {
		if (dfu_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->dfu->client);
	if (!device_info) {
		return -1;
	}

	*ecid = device_info->ecid;

	return 0;
}

int dfu_is_image4_supported(struct idevicerestore_client_t* client)
{
	if(client->dfu == NULL) {
		if (dfu_client_new(client) < 0) {
			return 0;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->dfu->client);
	if (!device_info) {
		return 0;
	}

	return (device_info->ibfl & IBOOT_FLAG_IMAGE4_AWARE);
}

int dfu_get_ap_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	if(client->dfu == NULL) {
		if (dfu_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->dfu->client);
	if (!device_info) {
		return -1;
	}

	if (device_info->ap_nonce && device_info->ap_nonce_size > 0) {
		*nonce = (unsigned char*)malloc(device_info->ap_nonce_size);
		if (!*nonce) {
			return -1;
		}
		*nonce_size = device_info->ap_nonce_size;
		memcpy(*nonce, device_info->ap_nonce, *nonce_size);
	}

	return 0;
}

int dfu_get_sep_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	if(client->dfu == NULL) {
		if (dfu_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->dfu->client);
	if (!device_info) {
		return -1;
	}

	if (device_info->sep_nonce && device_info->sep_nonce_size > 0) {
		*nonce = (unsigned char*)malloc(device_info->sep_nonce_size);
		if (!*nonce) {
			return -1;
		}
		*nonce_size = device_info->sep_nonce_size;
		memcpy(*nonce, device_info->sep_nonce, *nonce_size);
	}

	return 0;
}
