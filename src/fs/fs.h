#ifndef _CPTS360_FS_H
#define _CPTS360_FS_H

#include "../debug/debug.h"
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//// TYPEDEF ////

// define shorter TYPES for convenience
typedef struct ext2_group_desc group_desc;
typedef struct ext2_super_block super_block;
typedef struct ext2_inode inode;
typedef struct ext2_dir_entry_2 dir_entry;

//// CONST ////

// Block size
#define BLKSIZE_1024 1024

// Inode numbers of EXT2 as defined in ext2fs.h
// EXT2_BAD_INO 1         Bad blocks inode
// EXT2_ROOT_INO 2        Root inode
// EXT4_USR_QUOTA_INO 3   User quota inode
// EXT4_GRP_QUOTA_INO 4   Group quota inode
// EXT2_BOOT_LOADER_INO 5 Boot loader inode
// EXT2_UNDEL_DIR_INO 6   Undelete directory inode
// EXT2_RESIZE_INO 7      Reserved group descriptors inode
// EXT2_JOURNAL_INO 8     Journal inode
// EXT2_EXCLUDE_INO 9     The "exclude" inode, for snapshots
// EXT4_REPLICA_INO 10    Used by non-upstream feature

// File types
#define __S_IFDIR 0040000  /* Directory.  */
#define __S_IFCHR 0020000  /* Character device.  */
#define __S_IFBLK 0060000  /* Block device.  */
#define __S_IFREG 0100000  /* Regular file.  */
#define __S_IFIFO 0010000  /* FIFO.  */
#define __S_IFLNK 0120000  /* Symbolic link.  */
#define __S_IFSOCK 0140000 /* Socket.  */
#define DIR_FILE __S_IFDIR
#define REG_FILE __S_IFREG
#define LNK_FILE __S_IFLNK

// Proc status
#define PROC_FREE 0
#define PROC_BUSY 1

// file system table sizes
#define NUM_MINODES 100
#define NUM_MOUNT_ENTRIES 10
#define NUM_PROCS 2
#define NUM_FD 10
#define NUM_OFT 40

//// STRUCTS ////

// oft, proc, minode, mount_entry

// for parsing paths into
typedef struct path {
  char argv[4096][64]; // count of strings
  int argc;            // array of strings
  bool is_absolute;
  bool is_root;
} path;

// In-memory inodes structure
typedef struct minode {
  // disk inode
  inode inode;
  // inode number
  int ino;
  // use count
  int ref_count;
  // modified flag
  bool dirty;
  // mount entry pointer
  struct mount_entry *mount_entry;
  // ignored for simple FS
  // int lock;
} minode;

// Open file Table AKA opened file instance
typedef struct oft {
  // file mode
  int mode;
  // number of PROCs sharing this instance
  int ref_count;
  // pointer to minode of file
  minode *minode;
  // byte offset for R|W
  int offset;
} oft;

// PROC structure
typedef struct proc {
  struct proc *next;
  int pid;
  int uid;
  int gid;
  int ppid;
  int status;
  minode *cwd;
  oft *oft_arr[NUM_FD];
} proc;

// Mount Entry structure
typedef struct mount_entry {
  // mounted flag
  bool mounted;
  // device file descriptor
  int fd;
  // device root inode
  minode *root;
  // device path ex: ~/project/exampledisk
  char dev_path[64];
  // mount path ex: / for root, /A or /B or /C ... for non-root
  char mnt_path[64];
  // superblock
  super_block super_block;
  // group_desc
  group_desc group_desc;
} mount_entry;

// REMOVED
// bmap == dev_gd->bg_block_bitmap;
// imap == dev_gd->bg_inode_bitmap;
// iblock == dev_gd->bg_inode_table;

//// VAR ////

// in memory  inodes
minode minode_arr[NUM_MINODES];

// root mounted inode
minode *global_root_inode;

// mount tables
mount_entry mount_entry_arr[NUM_MOUNT_ENTRIES];

mount_entry *global_root_mount;

// Opened file instance
oft oft_arr[NUM_OFT];

// PROC structures
proc proc_arr[NUM_PROCS];

// current executing PROC
proc *running;

//// FUNC ////

// fs_minode
minode *alloc_minode();
bool free_minode(minode *);
minode *get_minode(mount_entry *, int);
bool put_minode(minode *);

// fs_mount
int fs_init();
mount_entry *mount_device(char *, char *);

// fs_path
int parse_path(char *, path *);
int search_dir(minode *, char *);
minode *search_path(path *);
int list_dir(minode *mip, dir_entry *);

// fs_util
bool check_mode(inode *, int);
bool get_block(mount_entry *, int, char *);
bool put_block(mount_entry *, int, char *);
bool tst_bit(char *, int);
bool set_bit(char *, int);
bool clr_bit(char *, int);
bool get_block(mount_entry *, int, char *);
int alloc_inode(mount_entry *);
int alloc_block(mount_entry *);
int add_dir_entry(minode *, dir_entry *);
#endif