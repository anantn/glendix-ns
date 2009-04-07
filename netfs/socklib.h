/*
 * Copyright 2009 The Glendix Project
 * Authors: Rahul Murmuria <rahul at murmuria dot net>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/string.h>

int socknet_create(struct dentry *, int, int, int);
int socknet_close(struct dentry *);
int socknet_connect(struct dentry *, struct sockaddr *, int);
ssize_t socknet_recv(struct file *, int);
ssize_t socknet_send(struct file *, int flags);


