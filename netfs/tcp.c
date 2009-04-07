/*
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#define __NO_VERSION__

#include "netfs.h"
/*
 * Create the files that we export.
 */

void tcp_create_files (struct super_block *sb, struct dentry *root)
{
 	static char *clone_tmp, *stats_tmp;
	struct dentry *subdir;
	
	subdir = slashnet_create_dir(sb, root, "tcp");
	if (subdir) {
		clone_tmp = kmalloc(TMPSIZE, GFP_KERNEL);
		memset (clone_tmp, 0, TMPSIZE);
		slashnet_create_file(sb, subdir, "clone", clone_tmp);

		stats_tmp = kmalloc(TMPSIZE, GFP_KERNEL);
		memset (stats_tmp, 0, TMPSIZE);
		slashnet_create_file(sb, subdir, "stats", stats_tmp);
/* 
 * NOTE: stat_tmp and clone_tmp should not be free'd here because
 * the same memory is being assigned to inode->-_private in the
 * main program. It is desirable to have this memory allotted for
 * every file as long as the filesystem is mount on a directory.
 */

	}
}

/*
 * Processing the written instructions
 */

/*
 * /net/tcp/clone
 */

void tcp_clone_process (struct file *filp)
{
	char *tmp;
	tmp = (char *)(filp->private_data);
	strcat(tmp, ":processed");
}

