#include "hw_config.h"
#include "sd_card.h"
#include "lfs.h"
//
#include "printf.h"

#define BLOCK_SZ 512

// variables used by the filesystem
lfs_t lfs;

void print_fs_err(int err) {
	switch (err) {
	case LFS_ERR_OK:
		printf(" No error \r\n");
		break;
	case LFS_ERR_IO:
		printf(" Error during device operation \r\n");
		break;
	case LFS_ERR_CORRUPT:
		printf(" Corrupted \r\n");
		break;
	case LFS_ERR_NOENT:
		printf(" No directory entry \r\n");
		break;
	case LFS_ERR_EXIST:
		printf(" Entry already exists \r\n");
		break;
	case LFS_ERR_NOTDIR:
		printf(" Entry is not a dir \r\n");
		break;
	case LFS_ERR_ISDIR:
		printf(" Entry is a dir \r\n");
		break;
	case LFS_ERR_NOTEMPTY:
		printf(" Dir is not empty \r\n");
		break;
	case LFS_ERR_BADF:
		printf(" Bad file number \r\n");
		break;
	case LFS_ERR_FBIG:
		printf(" File too large \r\n");
		break;
	case LFS_ERR_INVAL:
		printf(" Invalid parameter \r\n");
		break;
	case LFS_ERR_NOSPC:
		printf(" No space left on device \r\n");
		break;
	case LFS_ERR_NOMEM:
		printf(" No more memory available \r\n");
		break;
	case LFS_ERR_NOATTR:
		printf(" No data/attr available \r\n");
		break;
	case LFS_ERR_NAMETOOLONG:
		printf(" File name too long \r\n");
	}
}

// Read a region in a block. Negative error codes are propogated
// to the user.
static int user_provided_block_device_read(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
	configASSERT(0 == off);
	configASSERT(0 == size % BLOCK_SZ);
	return sd_read_blocks((sd_card_t*) c->context, buffer, block, size / BLOCK_SZ);
}
// Program a region in a block. The block must have previously
// been erased. Negative error codes are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
static int user_provided_block_device_prog(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
	configASSERT(0 == off);
	configASSERT(0 == size % BLOCK_SZ);
	return sd_write_blocks((sd_card_t*) c->context, buffer, block, size / BLOCK_SZ);
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
static int user_provided_block_device_erase(const struct lfs_config *c,
		lfs_block_t block) {
	(void)c;
	(void)block;
	return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propogated to the user.
static int user_provided_block_device_sync(const struct lfs_config *c) {
	(void)c;
	return 0;
}

//static uint8_t s_buffer[BLOCK_SZ],
//// Statically allocated read buffer Must be cache_size.
static uint8_t s_read_buffer[BLOCK_SZ];
// Statically allocated program buffer. Must be cache_size.
static uint8_t s_prog_buffer[BLOCK_SZ];
// Statically allocated lookahead buffer. Must be lookahead_size
// and aligned to a 32-bit boundary.
static uint32_t s_lookahead_buffer[4];

// configuration of the filesystem is provided by this struct
struct lfs_config cfg =
{
	// block device operations
	.read = user_provided_block_device_read,
	.prog = user_provided_block_device_prog,
	.erase = user_provided_block_device_erase,
	.sync = user_provided_block_device_sync,

	// block device configuration
	.block_size = BLOCK_SZ,
	.cache_size = BLOCK_SZ,
	.read_size = sizeof s_read_buffer,
	.prog_size = sizeof s_prog_buffer,
	.lookahead_size = sizeof s_lookahead_buffer,

	.read_buffer = s_read_buffer,
	.prog_buffer = s_prog_buffer,
	.lookahead_buffer = s_lookahead_buffer,

	.block_cycles = 500,
};

// Statically allocated file buffer. Must be cache_size.
static uint8_t file_buffer[BLOCK_SZ];

struct lfs_file_config file_cfg = {
		.buffer = file_buffer
};

bool fs_init() {
	// Initialize:
	sd_card_t *p_sd = sd_get_by_num(0);
	configASSERT(p_sd);
	int status = sd_init_card(p_sd);
	if (status & STA_NOINIT) /* 0x01 */
		printf("Drive not initialized\r\n");
	if (status & STA_NODISK) /* 0x02 */
		printf("No medium in the drive\r\n");
	if (status & STA_PROTECT) /* 0x04 */
		printf("Write protected\r\n");
	if (status & (STA_NOINIT | STA_NODISK))
		return false;

	cfg.context = p_sd;
	cfg.block_count = p_sd->sectors;
	return true;
}

// entry point
//int main(void) {
int fs_test(void) {
	lfs_file_t file;
	memset(&file, 0, sizeof file);

	if (!fs_init())
		return -1;

// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);

// reformat if we can't mount the filesystem
// this should only happen on the first boot
	if (err) {
		lfs_format(&lfs, &cfg);
		lfs_mount(&lfs, &cfg);
	}

// read current count
	uint32_t boot_count = 0;
	lfs_file_opencfg(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
	lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

// update boot count
	boot_count += 1;
	lfs_file_rewind(&lfs, &file);
	lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

// remember the storage is not updated until the file is closed successfully
	lfs_file_close(&lfs, &file);

// release any resources we were using
	lfs_unmount(&lfs);

// print the boot count
	printf("boot_count: %lu\r\n", boot_count);

	return 0;
}
