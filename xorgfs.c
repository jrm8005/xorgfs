/*
 * xorgfs -- A FUSE filesystem for exposing various X11 internal information
 *
 * AUTHOR: J.R. Mauro
 * 
 * Copyright (C) 2009 -- distributed under the terms of the GNU GPL Version 2
 *
 */

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "xcalls.h"

static const char *xorg_mouse = "/mouse";

static int xorg_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if(strcmp(path, xorg_mouse) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 14;
	}
	else
		res = -ENOENT;

	return res;
}

static int xorg_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, xorg_mouse + 1, NULL, 0);

	return 0;
}

static int xorg_open(const char *path, struct fuse_file_info *fi)
{
	if(strcmp(path, xorg_mouse) != 0)
		return -ENOENT;

	if((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int xorg_read(const char *path, char *buf, size_t size, off_t offset,
					  struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	char coords[12] = "";
	int x=0, y=0;

	if(strcmp(path, xorg_mouse) != 0)
		return -ENOENT;

	len = 14;
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		if (xorg_get_cursor_pos(&x, &y)) {
			sprintf(coords, "%s\n", "error");
		} else {
			sprintf(coords, "%3d %3d\n", x, y);
		}
		memcpy(buf, coords + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations xorg_oper = {
	.getattr = xorg_getattr,
	.readdir = xorg_readdir,
	.open = xorg_open,
	.read = xorg_read,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &xorg_oper);
}

