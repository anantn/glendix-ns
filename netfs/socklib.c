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

/* BSD socket wrapper */
int socknet_create(struct dentry *dir, int domain, int type, int protocol)
{
	struct socket *sock = NULL;
	int ret = 0;
	
	ret = sock_create(domain, type, protocol, &sock);
	dir->d_inode->i_private = sock;

	return ret;
}

int socknet_connect(struct dentry *dir, struct sockaddr *uaddr, int addr_len)
{
	struct socket *sock;
	int ret;

	sock = (struct socket *)(dir->d_inode->i_private);
	ret = sock->ops->connect(sock, uaddr, addr_len, sock->file->f_flags);
	
	return ret;
}


ssize_t socknet_recv(struct file *filp, int flags)
{
	struct socket *sock;
	struct msghdr msg;
	struct iovec iov;
	int ret;
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

