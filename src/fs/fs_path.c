#include "fs.h"

// set if path relative/absolute/root in buf
// split path_name on "/" into argv and argc of buf
// return argc
int parse_path(char *path_name, path *buf_path) {
  char *s, safe_path[256];
  buf_path->argc = 0;

  // check if root
  if (strcmp(path_name, "/") == 0) {
    buf_path->is_root = true;
    buf_path->argc = 1;
  } else
    buf_path->is_root = false;

  // check if absolute or relative
  if (path_name[0] == '/')
    buf_path->is_absolute = true;
  else
    buf_path->is_absolute = false;

  strcpy(safe_path, path_name);
  // split into components
  s = strtok(safe_path, "/");
  while (s) {
    strcpy(buf_path->argv[buf_path->argc++], s);
    s = strtok(NULL, "/");
  }
  buf_path->argv[buf_path->argc][0] = 0;
  return buf_path->argc;
}

// iterates through i_block of mip
// return ino of dir with dir_name on success
// return 0 on failure
int search_dir(minode *mip, char *dir_name) {
  int i;
  char *fs_p, temp[256], buf[BLKSIZE_1024] = {0}, *b = buf;
  dir_entry *dep;
  DEBUG_PRINT("search for %s\n", dir_name);
  if (!S_ISDIR(mip->inode.i_mode)) {
    DEBUG_PRINT("search fail %s is not a dir\n", dir_name);
    return 0;
  }
  // search dir_entry direct blocks only
  for (i = 0; i < 12; i++) {
    // if direct block is null stap
    if (mip->inode.i_block[i] == 0)
      return 0;
    // get next direct block
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dep = (dir_entry *)buf;
    fs_p = buf;
    while (fs_p < buf + BLKSIZE_1024) {
      snprintf(temp, dep->name_len + 1, "%s", dep->name);
      DEBUG_PRINT("ino:%d rec_len:%d name_len:%u name:%s\n", dep->inode,
                  dep->rec_len, dep->name_len, temp);
      if (strcmp(dir_name, temp) == 0) {
        DEBUG_PRINT("found %s : inumber = %d\n", dir_name, dep->inode);
        return dep->inode;
      }
      fs_p += dep->rec_len;
      dep = (dir_entry *)fs_p;
    }
  }
  return 0;
}

// iterate through i_block of mip and store in dir_arr
// return dirc on success, return 0 on failure
// todo: indirect and double indirect
int list_dir(minode *mip, dir_entry *dir_arr) {
  char *fs_p, buf[BLKSIZE_1024];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode)) {
    DEBUG_PRINT("list fail ino %d is not a dir\n", mip->ino);
    return 0;
  }
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    fs_p = buf;
    // todo: double check this condition
    while (fs_p < buf + BLKSIZE_1024) {
      dirp = (dir_entry *)fs_p;
      dir_arr[dirc] = *dirp;
      dirc++;
      fs_p += dirp->rec_len;
    }
  }
  return dirc;
}

// returns count of dir entries in mip on success
// returns 0 on failure
int count_dir(minode *mip) {
  char buf[BLKSIZE_1024], *bufp = buf, temp[256];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode))
    return 0;
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    bufp = buf;
    // todo: double check this condition
    while (bufp < buf + BLKSIZE_1024) {
      snprintf(temp, dirp->name_len + 1, "%s", dirp->name);
      DEBUG_PRINT("ino:%d rec_len:%d name_len:%u name:%s\n", dirp->inode,
                  dirp->rec_len, dirp->name_len, temp);
      dirc++;
      bufp += dirp->rec_len;
      dirp = (dir_entry *)bufp;
    }
  }
  return dirc;
}

// returns minode of path on success
// returns NULL on failure
// does not put found minode
minode *search_path(path *target_path) {
  path safe_path = *target_path;
  minode *mip;
  int i, ino;
  if (target_path->is_root)
    return global_root_inode;
  if (target_path->is_absolute)
    mip = global_root_inode; // if absolute
  else
    mip = running->cwd; // if relative
  mip->ref_count++;

  // search for each token string
  for (i = 0; i < safe_path.argc; i++) {
    if (S_ISLNK(mip->inode.i_mode)) { // handle symlink
      path sym_path;
      parse_path((char *)mip->inode.i_block, &sym_path);
      if (sym_path.is_absolute) // recurse and continue iteration on new mip
        mip = search_path(&sym_path);
      else // append sym_path to target_path and continue iteration
      {
        // TODO: WATCH THIS FUCK-RIDDLED BAD BOY RUN
        memcpy(&safe_path.argv[i + sym_path.argc], &safe_path.argv[i],
               sizeof(char *) * sym_path.argc);
        memcpy(&safe_path.argv[i], &sym_path.argv,
               sizeof(char *) * sym_path.argc);
        safe_path.argc += sym_path.argc;
      }
    } else {
      ino = search_dir(mip, safe_path.argv[i]);

      if (!ino) {
        printf("no such component name %s\n", target_path->argv[i]);
        put_minode(mip);
        return NULL;
      }
      put_minode(mip);
      // switch to new minode
      mip = get_minode(mip->mount_entry, ino);
    }
  }
  return mip;
}
