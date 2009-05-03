/*
 * Copyright 2009 The Glendix Project
 * Author(s): Rahul Murmuria <rahul at murmuria dot in>
 * This file may be redistributed under the terms of the GNU GPL.
 * 
 * Code design courtesy of http://lwn.net
 */

#include "netfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul Murmuria <rahul at murmuria dot in>");

inline unsigned int blksize_bits(unsigned int size)
{
    unsigned int bits = 8;
    do {
        bits++;
        size >>= 1;
    } while (size > 256);
    return bits;
}

static struct inode *slashnet_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);

	if (ret) {
		ret->i_mode = mode;
		ret->i_uid = ret->i_gid = 0;
		ret->i_blkbits = blksize_bits(PAGE_CACHE_SIZE);
		ret->i_blocks = 0;
		ret->i_atime = ret->i_mtime = ret->i_ctime = CURRENT_TIME;
	}
	return ret;
}


/*
 * The operations on our "files".
 */

/*
 * Open a file.
 */
static int slashnet_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

/*
 * Read a file.
 */
static ssize_t slashnet_read_file(struct file *filp, char *dnsquery,
		size_t count, loff_t *offset)
{
	int len, retval;
	len = strlen(filp->private_data);
	if (*offset > len)
		return 0;
	if (count > len - *offset)
		count = len - *offset;
	
	retval = copy_to_user(dnsquery, filp->private_data + *offset, count);

	if (retval)
		return -EFAULT;
	
	*offset += count;
	return count;
}

/*
 * Write a file.
 */
static ssize_t slashnet_write_file(struct file *filp, const char *dnsquery,
		size_t count, loff_t *offset)
{
	char *tmp;
	tmp = (char *)(filp->private_data);
	
	if (*offset != 0)
		return -EINVAL;
	
	memset(tmp, 0, TMPSIZE);
	if(copy_from_user(tmp, dnsquery, count))
		return -EFAULT;
	tmp[count-1] = '\0';
	
	if (!strcmp(filp->f_dentry->d_name.name, "clone") && 
		!strcmp(filp->f_dentry->d_parent->d_name.name, "tcp"))
		tcp_clone_process(filp);
	else if (!strcmp(filp->f_dentry->d_name.name, "ctl") &&
		!strcmp(filp->f_dentry->d_parent->d_parent->d_name.name,"tcp"))
		tcp_n_ctl_process(filp);
	else if (!strcmp(filp->f_dentry->d_name.name, "data") &&
		!strcmp(filp->f_dentry->d_parent->d_parent->d_name.name,"tcp"))
		tcp_n_data_process(filp);
	else if (!strcmp(filp->f_dentry->d_name.name, "cs") && 
		!strcmp(filp->f_dentry->d_parent->d_name.name, "/"))
		slashnet_cs_process(filp);

	/* debug */
//	printk("*** %s/%s: %s ***\n", filp->f_dentry->d_parent->d_name.name,
//		filp->f_dentry->d_name.name, (char *) filp->private_data);
	
	return count;
}


/*
 * Now we can put together our file operations structure.
 */
static struct file_operations slashnet_file_ops = {
	.open	= slashnet_open,
	.read 	= slashnet_read_file,
	.write  = slashnet_write_file,
};

/*
 * Create a file.
 */
struct dentry *slashnet_create_file (struct super_block *sb,
		struct dentry *dir, const char *name, char *initval)
{
	struct dentry *dentry;
	struct inode *inode;
	struct qstr qname;
/*
 * Make a hashed version of the name to go with the dentry.
 */
	qname.name = name;
	qname.len = strlen (name);
	qname.hash = full_name_hash(name, qname.len);
/*
 * Now we can create our dentry and the inode to go with it.
 */
	dentry = d_alloc(dir, &qname);
	if (! dentry)
		goto out;
	inode = slashnet_make_inode(sb, S_IFREG | 0644);
	if (! inode)
		goto out_dput;
	inode->i_fop = &slashnet_file_ops;
	inode->i_private = initval;

/*
 * Put it all into the dentry cache and we're done.
 */
	d_add(dentry, inode);
	return dentry;
/*
 * Then again, maybe it didn't work.
 */
  out_dput:
	dput(dentry);
  out:
	return 0;
}


/*
 * Create a directory which can be used to hold files.  This code is
 * almost identical to the "create file" logic, except that we create
 * the inode with a different mode, and use the libfs "simple" operations.
 */
struct dentry *slashnet_create_dir (struct super_block *sb,
		struct dentry *parent, const char *name)
{
	struct dentry *dentry;
	struct inode *inode;
	struct qstr qname;

	qname.name = name;
	qname.len = strlen (name);
	qname.hash = full_name_hash(name, qname.len);
	dentry = d_alloc(parent, &qname);
	if (! dentry)
		goto out;

	inode = slashnet_make_inode(sb, S_IFDIR | 0644);
	if (! inode)
		goto out_dput;
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &simple_dir_operations;

	d_add(dentry, inode);
	return dentry;

  out_dput:
	dput(dentry);
  out:
	return 0;
}


/*
 * Function to create new connection folder named n
 * to be called when the net folder n-1 is busy.
 */

struct dentry *slashnet_create_netfolder(struct super_block *sb, 
				struct dentry *dir, const char *n)
{
	static char *ctl_tmp, *data_tmp;
	struct dentry *subdir;

	subdir = slashnet_create_dir(sb, dir, n);
	if (subdir) {
		ctl_tmp = kmalloc(TMPSIZE, GFP_KERNEL);
		memset (ctl_tmp, 0, TMPSIZE);
		slashnet_create_file(sb, subdir, "ctl", ctl_tmp);

		data_tmp = kmalloc(TMPSIZE, GFP_KERNEL);
		memset (data_tmp, 0, TMPSIZE);
		slashnet_create_file(sb, subdir, "data", data_tmp);
	}

	return subdir;
}


/*
 * Create the files that we export.
 */

static void slashnet_create_files (struct super_block *sb, struct dentry *root)
{
	cs_create_files (sb, root);
	tcp_create_files (sb, root);
	udp_create_files (sb, root);
	ether_create_files (sb, root);
}


/*
 * Superblock stuff.  This is all boilerplate to give the vfs something
 * that looks like a filesystem to work with.
 */

/*
 * Our superblock operations, both of which are generic kernel ops
 * that we don't have to write ourselves.
 */
static struct super_operations slashnet_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};

/*
 * "Fill" a superblock with mundane stuff.
 */
static int slashnet_fill_super (struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	struct dentry *root_dentry;
/*
 * Basic parameters.
 */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = NET_MAGIC;
	sb->s_op = &slashnet_s_ops;
/*
 * We need to conjure up an inode to represent the root directory
 * of this filesystem.  Its operations all come from libfs, so we
 * don't have to mess with actually *doing* things inside this
 * directory.
 */
	root = slashnet_make_inode (sb, S_IFDIR | 0755);
	if (! root)
		goto out;
	root->i_op = &simple_dir_inode_operations;
	root->i_fop = &simple_dir_operations;
/*
 * Get a dentry to represent the directory in core.
 */
	root_dentry = d_alloc_root(root);
	if (! root_dentry)
		goto out_iput;
	sb->s_root = root_dentry;
/*
 * Make up the files which will be in this filesystem, and we're done.
 */
	slashnet_create_files (sb, root_dentry);
	return 0;
	
  out_iput:
	iput(root);
  out:
	return -ENOMEM;
}


/*
 * Stuff to pass in when registering the filesystem.
 */
static int slashnet_get_super(struct file_system_type *fst, 
		int flags, const char *devname, void *data, 
		struct vfsmount *mnt)
{
	return get_sb_single(fst, flags, data, slashnet_fill_super, mnt);
}

static struct file_system_type slashnet_type = {
	.owner 		= THIS_MODULE,
	.name		= "net",
	.get_sb		= slashnet_get_super,
	.kill_sb	= kill_litter_super,
};




/*
 * Get things set up.
 */
static int __init slashnet_init(void)
{
	return register_filesystem(&slashnet_type);
}

static void __exit slashnet_exit(void)
{
	unregister_filesystem(&slashnet_type);
}

module_init(slashnet_init);
module_exit(slashnet_exit);

