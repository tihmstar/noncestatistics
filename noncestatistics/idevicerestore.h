/*
 * idevicerestore.h
 * Restore device firmware and filesystem
 *
 * Copyright (c) 2010-2012 Martin Szulecki. All Rights Reserved.
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

#ifndef IDEVICERESTORE_H
#define IDEVICERESTORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <plist/plist.h>

// the flag with value 1 is reserved for internal use only. don't use it.
#define FLAG_DEBUG           1 << 1
#define FLAG_ERASE           1 << 2
#define FLAG_CUSTOM          1 << 3
#define FLAG_EXCLUDE         1 << 4
#define FLAG_PWN             1 << 5
#define FLAG_NOACTION        1 << 6
#define FLAG_SHSHONLY        1 << 7
#define FLAG_LATEST          1 << 8
#define FLAG_DOWNGRADE       1 << 9
#define FLAG_OTAMANIFEST     1 << 10
#define FLAG_BOOT            1 << 11
#define FLAG_PANICLOG        1 << 12
#define FLAG_NOBOOTX         1 << 13

struct idevicerestore_client_t;

enum {
	RESTORE_STEP_DETECT = 0,
	RESTORE_STEP_PREPARE,
	RESTORE_STEP_UPLOAD_FS,
	RESTORE_STEP_VERIFY_FS,
	RESTORE_STEP_FLASH_FW,
	RESTORE_STEP_FLASH_BB,
	RESTORE_NUM_STEPS
};

typedef void (*idevicerestore_progress_cb_t)(int step, double step_progress, void* userdata);

struct idevicerestore_client_t* idevicerestore_client_new(void);
void idevicerestore_client_free(struct idevicerestore_client_t* client);

int idevicerestore_start(struct idevicerestore_client_t* client);

int check_mode(struct idevicerestore_client_t* client);
const char* check_hardware_model(struct idevicerestore_client_t* client);
int get_ecid(struct idevicerestore_client_t* client, uint64_t* ecid);
int get_ap_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size);
int get_sep_nonce(struct idevicerestore_client_t* client, unsigned char** nonce, int* nonce_size);
#ifdef __cplusplus
}
#endif

#endif
