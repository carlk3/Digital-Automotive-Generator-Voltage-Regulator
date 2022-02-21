/*
 * fs.h
 *
 *  Created on: Dec 22, 2021
 *      Author: carlk
 */

#ifndef INC_FS_H_
#define INC_FS_H_

#include "lfs.h"

extern lfs_t lfs;
extern struct lfs_config cfg;
extern struct lfs_file_config file_cfg;

void print_fs_err(const char *msg, int err);
bool fs_init();
int fs_test();

#endif /* INC_FS_H_ */
