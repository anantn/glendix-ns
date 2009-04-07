/*
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#define __NO_VERSION__

#include "netfs.h"
/*
 * Create the files that we export.
 */

void cs_create_files (struct super_block *sb, struct dentry *root)
{
 	static char *cs_tmp;
	cs_tmp = kmalloc(TMPSIZE, GFP_KERNEL);
	memset (cs_tmp, 0, TMPSIZE);
	slashnet_create_file(sb, root, "cs", cs_tmp);
/* 
 * NOTE: cs_tmp should not be free'd here because the same memory 
 * is being assigned to inode->-_private in the main program. 
 * It is desirable to have this memory allotted for every file 
 * as long as the filesystem is mount on a directory.
 */

}

/*
 * Processing the written instructions
 */

void slashnet_cs_process (struct file *filp)
{
	char *tmp;
	tmp = (char *)(filp->private_data);
	strcat(tmp, ":processed");
}


