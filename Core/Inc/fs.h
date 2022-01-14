/*
 * fs.h
 *
 *  Created on: Dec 22, 2021
 *      Author: carlk
 */

#ifndef INC_FS_H_
#define INC_FS_H_

#include "lfs.h"

void print_fs_err(int err);
int user_provided_block_device_read(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int user_provided_block_device_prog(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int user_provided_block_device_erase(const struct lfs_config *c,
		lfs_block_t block);
int user_provided_block_device_sync(const struct lfs_config *c);

int fs_test(void);

#endif /* INC_FS_H_ */
