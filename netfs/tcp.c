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
 * the same memory is being assigned to inode->i_private in the
 * main program. It is desirable to have this memory allotted for
 * every file as long as the filesystem is mount on a directory.
 */

	}
	slashnet_create_netfolder(sb, subdir, "0");
}

/*
 * Processing the written instructions
 */

/*
 * write on /net/tcp/clone
 */

void tcp_clone_process (struct file *filp)
{
	char *tmp;
	tmp = (char *)(filp->private_data);
	strcat(tmp, ":processed");
}

/*
 * write on /net/tcp/n/ctl
 */

void tcp_n_ctl_process (struct file *filp)
{
	char *tmp, *ip, *ip_tmp, *port;
	int tmp_size, counter, ret;
	struct dentry *parent;

	parent = filp->f_dentry->d_parent;
	tmp = (char *)(filp->private_data);
	tmp_size = strlen(tmp);

	if(!strncmp(tmp, "connect ", 8)) {
		tmp = tmp + 8;
		ip = kmalloc (tmp_size-8, GFP_KERNEL);
		memset (ip, 0, tmp_size-8);
		ip_tmp = ip;
		for (counter=0; counter < tmp_size-8; counter++) {
			if (*tmp == ':' || *tmp == '!') break;
			*ip_tmp = *tmp;
			ip_tmp++;
			tmp++;
		}
		*ip_tmp = '\0';

		tmp++;
		port = kmalloc (6, GFP_KERNEL);
		memset (port, 0, 6);
		memcpy (port, tmp, 6);
		port[5] = '\0';
		/* debug */
		printk("***Connecting to %s port %s ***\n", ip, port);

		ret = socknet_connect(parent, AF_INET, SOCK_STREAM, 0,
			ip, port);
		if(ret<0) 
			printk("Connection to %s failed!", ip);
	}
}

void tcp_n_data_process (struct file *filp)
{
	char *tmp;
	tmp = (char *)(filp->private_data);
	strcat(tmp, ": received");
}

