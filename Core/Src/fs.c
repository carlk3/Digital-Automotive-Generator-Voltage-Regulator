#include "hw_config.h"
#include "sd_card.h"
#include "lfs.h"
//
#include "printf.h"

// variables used by the filesystem
static lfs_t lfs;
static lfs_file_t file;

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
int user_provided_block_device_read(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
	configASSERT(0 == off);
	configASSERT(0 == size % 512);
	return sd_read_blocks((sd_card_t*) c->context, buffer, block, size / 512);
}
// Program a region in a block. The block must have previously
// been erased. Negative error codes are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int user_provided_block_device_prog(const struct lfs_config *c,
		lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
	configASSERT(0 == off);
	configASSERT(0 == size % 512);
	return sd_write_blocks((sd_card_t*) c->context, buffer, block, size / 512);
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int user_provided_block_device_erase(const struct lfs_config *c,
		lfs_block_t block) {
	(void)c;
	(void)block;
	return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propogated to the user.
int user_provided_block_device_sync(const struct lfs_config *c) {
	(void)c;
	return 0;
}

// configuration of the filesystem is provided by this struct
static struct lfs_config cfg =
{
	// block device operations
	.read = user_provided_block_device_read,
	.prog = user_provided_block_device_prog,
	.erase = user_provided_block_device_erase,
	.sync = user_provided_block_device_sync,

	// block device configuration
	.read_size = 512,
	.prog_size = 512,
	.block_size = 512,
//	.block_count = sd_sectors(),
	.cache_size = 512,
	.lookahead_size = 16,
	.block_cycles = 500,
};

// entry point
//int main(void) {
int fs_test(void) {
//
//	if (!sd_init_driver())
//		return -1;

	sd_card_t *p_sd = sd_get_by_num(0);
	configASSERT(p_sd);
	int status = sd_init_card(p_sd);
	switch (status) {
	case STA_NOINIT: /* 0x01 */
		printf("Drive not initialized\r\n");
		return -2;
	case STA_NODISK: /* 0x02 */
		printf("No medium in the drive\r\n");
		return -3;
	case STA_PROTECT: /* 0x04 */
		printf("Write protected\r\n");
		return -4;
	}
	cfg.context = p_sd;
	cfg.block_count = p_sd->sectors;

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
	lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
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
