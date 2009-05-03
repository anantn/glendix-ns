/* 
 * Copyright 2009 The Glendix Project
 * Author(s): Rahul Murmuria <rahul at murmuria dot in>
 *
 * Code design courtesy of the ksocket project.
 */

/*
 * Socket wrapper that provides network stack interface to netfs
 * Scheduled for replacement with fsnetlib in near future.
 */


#include "socklib.h"

/* 
 * BSD socket wrapper 
 */

/*
 * Create a new socket
 */
int socknet_create(struct dentry *dir, int domain, int type, int protocol)
{
	struct socket *sock = NULL;
	int ret = 0;
	
	ret = sock_create(domain, type, protocol, &sock);
	dir->d_inode->i_private = sock;

	return ret;
}

/*
 * Create a new socket and connect : Client-side-only code
 */
int socknet_connect(struct dentry *dir, int domain, int type, 
		int protocol, char *ip, char *port)
{
	struct socket *sock = NULL;
	struct sockaddr_in addr;
	int ret = 0, in_port;
	
	ret = sock_create(domain, type, protocol, &sock);
	if (ret < 0) {
		printk("Error sock_create\n");
		return ret;
	}

	dir->d_inode->i_private = sock;
	printk("Kernel socket created on directory %s/%s\n", 
			(dir->d_parent->d_name.name), (dir->d_name.name));
	
	addr.sin_family = domain;
	in_port = in_aton(port);
	addr.sin_port = htons(in_port);
	addr.sin_addr.s_addr = in_aton(ip);
	
	ret = sock->ops->connect(sock, (struct sockaddr *)&addr, 
			sizeof(addr), sock->file->f_flags);
	if (ret < 0) {
		printk("Error sock->ops->connect\n");
	} else {
		printk("Got connected!\n");
	}
	
	return ret;
}


ssize_t socknet_recv(struct file *filp, int flags)
{
	struct socket *sock;
	struct msghdr msg;
	struct iovec iov;
	int ret, length;
	mm_segment_t old_fs;

	sock = (struct socket *)(filp->f_dentry->d_parent->d_inode->i_private);

	length = strlen(filp->private_data);
	iov.iov_base = (void *)(filp->private_data);
	iov.iov_len = (__kernel_size_t)length;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sock_recvmsg(sock, &msg, length, flags);
	set_fs(old_fs);
	
	return ret;
}

ssize_t socknet_send(struct file *filp, int flags)
{
	struct socket *sock;
	struct msghdr msg;
	struct iovec iov;
	int len;
	size_t length;
	mm_segment_t old_fs;

	sock = (struct socket *)(filp->f_dentry->d_parent->d_inode->i_private);
	
	length = strlen(filp->private_data);
	iov.iov_base = (void *)(filp->private_data);
	iov.iov_len = (__kernel_size_t)length;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;

	msg.msg_flags = flags;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	len = sock_sendmsg(sock, &msg, length);
	set_fs(old_fs);
	
	return len;
}

int socknet_close(struct dentry *dir)
{
	struct socket *sock;
	int ret;

	sock = (struct socket *)(dir->d_inode->i_private);
	ret = sock->ops->release(sock);

	if (sock)
		sock_release(sock);

	return ret;
}

