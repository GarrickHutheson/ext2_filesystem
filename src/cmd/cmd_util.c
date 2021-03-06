#include "../debug/debug.h"
#include "cmd.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool do_cmd(cmd *c) {
  if (!c)
    DEBUG_PRINT("cmd was null\n");
  if (!strcmp(c->argv[0], "blocks")) {
    do_blocks(c);
  } else if (!strcmp(c->argv[0], "cd")) {
    do_cd(c);
  } else if (!strcmp(c->argv[0], "cat")) {
    do_cat(c);
  } else if (!strcmp(c->argv[0], "chmod")) {
    do_chmod(c);
  } else if (!strcmp(c->argv[0], "close")) {
    do_close(c);
  } else if (!strcmp(c->argv[0], "cp")) {
    do_cp(c);
  } else if (!strcmp(c->argv[0], "creat")) {
    do_creat(c);
  } else if (!strcmp(c->argv[0], "link")) {
    do_link(c);
  } else if (!strcmp(c->argv[0], "ls")) {
    do_ls(c);
  } else if (!strcmp(c->argv[0], "lseek")) {
    do_lseek(c);
  } else if (!strcmp(c->argv[0], "mkdir")) {
    do_mkdir(c);
  } else if (!strcmp(c->argv[0], "mount")) {
    do_mount(c);
  } else if (!strcmp(c->argv[0], "mv")) {
    do_mv(c);
  } else if (!strcmp(c->argv[0], "open")) {
    do_open(c);
  } else if (!strcmp(c->argv[0], "pwd")) {
    do_pwd(c);
  } else if (!strcmp(c->argv[0], "read")) {
    do_read(c);
  } else if (!strcmp(c->argv[0], "rmdir")) {
    do_rmdir(c);
  } else if (!strcmp(c->argv[0], "stat")) {
    do_stat(c);
  } else if (!strcmp(c->argv[0], "su")) {
    do_su(c);
  } else if (!strcmp(c->argv[0], "symlink")) {
    do_symlink(c);
  } else if (!strcmp(c->argv[0], "touch")) {
    do_touch(c);
  } else if (!strcmp(c->argv[0], "umount")) {
    do_umount(c);
  } else if (!strcmp(c->argv[0], "unlink")) {
    do_unlink(c);
  } else if (!strcmp(c->argv[0], "write")) {
    do_write(c);
  } else if (!strcmp(c->argv[0], "quit")) {
    for (int i = 0; i < NUM_MINODES; i++) {
      if (minode_arr[i].ref_count) {
        minode_arr[i].ref_count = 1;
        put_minode(&minode_arr[i]);
      }
    }
    for (int i = 0; i < NUM_MOUNT_ENTRIES; i++) {
      if (mount_entry_arr[i].fd)
        _umount(mount_entry_arr[i].mnt_path);
    }
    exit(0);
  } else {
    printf("command not recognized: %s\n", c->argv[0]);
  }
  return 0;
}

int parse_cmd(char *line, cmd *c) {
  // split by whitespace into cmd struct
  int i = 0;
  char *s = strtok(line, " ");
  for (; s; i++) {
    c->argv[i] = s;
    s = strtok(NULL, " ");
  }
  c->argc = i;
  // NULL terminate argv
  c->argv[i] = NULL;
  return i;
}
