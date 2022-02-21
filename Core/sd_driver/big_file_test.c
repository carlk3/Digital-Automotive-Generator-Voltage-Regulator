/* big_file_test.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include "cmsis_os.h"
//
#include "fs.h"
//
#include "printf.h"

#define FF_MAX_SS 512
#define BUFFSZ 8 * 1024

typedef uint32_t DWORD;
typedef unsigned int UINT;

// Create a file of size "size" bytes filled with random data seeded with "seed"
// static
static bool create_big_file_v1(const char *const pathname, size_t size,
                        unsigned seed) {
    lfs_file_t data_file;
    int val;

    assert(0 == size % sizeof(int));

    srand(seed);

	// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);
	if (LFS_ERR_OK != err)
		print_fs_err("mount", err);
	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err) {
		err = lfs_format(&lfs, &cfg);
		if (LFS_ERR_OK != err)
			print_fs_err("format", err);
		err = lfs_mount(&lfs, &cfg);
		if (LFS_ERR_OK != err) {
			print_fs_err("mount attempt after format", err);
			return false;
		}
	}
    /* Open the file, creating the file if it does not already exist. */
	err = lfs_file_opencfg(&lfs, &data_file, pathname,
			LFS_O_WRONLY | LFS_O_CREAT, &file_cfg);
	if (LFS_ERR_OK != err) {
		print_fs_err("lfs_file_opencfg", err);
		return false;
	}
    printf("Writing...\n");
    uint32_t xStart = osKernelGetTickCount();

    size_t i;
    for (i = 0; i < size / sizeof(val); ++i) {
        val = rand();
    	lfs_ssize_t nfw = lfs_file_write(&lfs, &data_file, &val, sizeof(val));
    	if (nfw < 0) {
    		print_fs_err("write", nfw);
    		return false;
    	} else {
    		configASSERT(sizeof(val) == nfw);
    	}
    }
    /* Close the file. */
	err = lfs_file_close(&lfs, &data_file);
	if (LFS_ERR_OK != err) {
		print_fs_err("close", err);
		return false;
	}
	uint32_t elapsed_ms = osKernelGetTickCount() - xStart;
    float elapsed = elapsed_ms / 1E3;
    printf("Elapsed seconds %.3g\n", elapsed);
    return true;
}

// Read a file of size "size" bytes filled with random data seeded with "seed"
// and verify the data
// static
static void check_big_file_v1(const char *const pathname, size_t size, uint32_t seed) {
    lfs_file_t data_file;

    assert(0 == size % sizeof(int));

    srand(seed);

    /* Open the file, creating the file if it does not already exist. */
	int err = lfs_file_opencfg(&lfs, &data_file, pathname,
			LFS_O_RDONLY, &file_cfg);
	if (LFS_ERR_OK != err) {
		print_fs_err("lfs_file_opencfg", err);
		return;
	}
    printf("Reading...\n");
    uint32_t xStart = osKernelGetTickCount();

    size_t i;
    int val;
    for (i = 0; i < size / sizeof(val); ++i) {
    	lfs_ssize_t nfr = lfs_file_read(&lfs, &data_file, &val, sizeof(val));
    	if (nfr < 0) {
    		print_fs_err("read", nfr);
    		return;
    	} else {
    		configASSERT(sizeof(val) == nfr);
    	}
        /* Check the buffer is filled with the expected data. */
        int expected = rand();
        if (val != expected)
            printf("Data mismatch at word %zu: expected=%d val=%d\n", i,
                   expected, val);
    }
    /* Close the file. */
	err = lfs_file_close(&lfs, &data_file);
	if (LFS_ERR_OK != err) {
		print_fs_err("close", err);
		return;
	}
	uint32_t elapsed_ms = osKernelGetTickCount() - xStart;
    float elapsed = elapsed_ms / 1E3;
    printf("Elapsed seconds %.3g\n", elapsed);
}

void big_file_test(const char *const pathname, size_t size, uint32_t seed) {
    if (create_big_file_v1(pathname, size, seed))
        check_big_file_v1(pathname, size, seed);
}

/* [] END OF FILE */
