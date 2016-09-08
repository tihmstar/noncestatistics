/*
 * recovery.c
 * Functions for handling idevices in recovery mode
 *
 * Copyright (c) 2010-2012 Martin Szulecki. All Rights Reserved.
 * Copyright (c) 2012 Nikias Bassen. All Rights Reserved.
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
#include <libirecovery.h>
#include <libimobiledevice/restore.h>
#include <libimobiledevice/libimobiledevice.h>

#include "idevicerestore.h"
#include "recovery.h"

int recovery_progress_callback(irecv_client_t client, const irecv_event_t* event) {
	if (event->type == IRECV_PROGRESS) {
		//print_progress_bar(event->progress);
	}
	return 0;
}

void recovery_client_free(struct idevicerestore_client_t* client) {
	if(client) {
		if (client->recovery) {
			if(client->recovery->client) {
				irecv_close(client->recovery->client);
				client->recovery->client = NULL;
			}
			free(client->recovery);
			client->recovery = NULL;
		}
	}
}

int recovery_client_new(struct idevicerestore_client_t* client) {
	int i = 0;
	int attempts = 20000;
	irecv_client_t recovery = NULL;
	irecv_error_t recovery_error = IRECV_E_UNKNOWN_ERROR;

	if(client->recovery == NULL) {
		client->recovery = (struct recovery_client_t*)malloc(sizeof(struct recovery_client_t));
		if (client->recovery == NULL) {
			error("ERROR: Out of memory\n");
			return -1;
		}
		memset(client->recovery, 0, sizeof(struct recovery_client_t));
	}

	//for (i = 1; i <= attempts; i++) {
    while(1){
		recovery_error = irecv_open_with_ecid(&recovery, client->ecid);
		if (recovery_error == IRECV_E_SUCCESS) {
			break;
		}

		if (i >= attempts) {
			error("ERROR: Unable to connect to device in recovery mode\n");
			return -1;
		}

		usleep(50000);
		debug("Retrying connection...\n");
	}

	if (client->srnm == NULL) {
		const struct irecv_device_info *device_info = irecv_get_device_info(recovery);
		if (device_info && device_info->srnm) {
			client->srnm = strdup(device_info->srnm);
			info("INFO: device serial number is %s\n", client->srnm);
		}
	}

	irecv_event_subscribe(recovery, IRECV_PROGRESS, &recovery_progress_callback, NULL);
	client->recovery->client = recovery;
	return 0;
}

int recovery_check_mode(struct idevicerestore_client_t* client) {
	irecv_client_t recovery = NULL;
	irecv_error_t recovery_error = IRECV_E_SUCCESS;
	int mode = 0;

	irecv_init();
	recovery_error=irecv_open_with_ecid(&recovery, client->ecid);

	if (recovery_error != IRECV_E_SUCCESS) {
		return -1;
	}

	irecv_get_mode(recovery, &mode);

	if ((mode == IRECV_K_DFU_MODE) || (mode == IRECV_K_WTF_MODE)) {
		irecv_close(recovery);
		return -1;
	}

	irecv_close(recovery);
	recovery = NULL;

	return 0;
}


int recovery_get_ecid(struct idevicerestore_client_t* client, uint64_t* ecid) {
	if(client->recovery == NULL) {
		if (recovery_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->recovery->client);
	if (!device_info) {
		return -1;
	}

	*ecid = device_info->ecid;

	return 0;
}

int recovery_get_ap_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	if(client->recovery == NULL) {
		if (recovery_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->recovery->client);
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

int recovery_get_sep_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size) {
	if(client->recovery == NULL) {
		if (recovery_client_new(client) < 0) {
			return -1;
		}
	}

	const struct irecv_device_info *device_info = irecv_get_device_info(client->recovery->client);
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

int recovery_send_reset(struct idevicerestore_client_t* client)
{
	irecv_send_command(client->recovery->client, "reset");
	return 0;
}

int recovery_set_autoboot(struct idevicerestore_client_t* client, int enable) {
    irecv_error_t recovery_error = IRECV_E_SUCCESS;
    
    recovery_error = irecv_send_command(client->recovery->client, (enable) ? "setenv auto-boot true" : "setenv auto-boot false");
    if (recovery_error != IRECV_E_SUCCESS) {
        error("ERROR: Unable to set auto-boot environmental variable\n");
        return -1;
    }
    
    recovery_error = irecv_send_command(client->recovery->client, "saveenv");
    if (recovery_error != IRECV_E_SUCCESS) {
        error("ERROR: Unable to save environmental variable\n");
        return -1;
    }
    
    return 0;
}





