/*
 * Copyright 2009 The Glendix Project
 * Author(s): Rahul Murmuria <rahul at murmuria dot in>
 *
 * The code to hook commands from netfs straight on to the sk structure 
 * interface of the transport layer. This code bypasses the sock_ioctl
 * and the socket structure, but is not yet completed for use.
 */

#define __NO_VERSION__

#include "net.h"
#include <net/sock.h>        	/* struct sock and operations */
#include <linux/socket.h>   	/* AF_UNSPEC */
#include <linux/err.h>
#include <linux/sched.h> 	/* TASK_INTERRUPTIBLE */
#include <timer.h>		/* DEFINE_WAIT, finish_wait */
#include <net/inet_common.h>   	/* inet_sock_destruct */
#include <net/protocol.h> 	/* INET_PROTOSW_REUSE */

/*
 * Create the sk structure
 */
int fsnet_create(struct net *net, struct file *filp, int protocol)
{
	struct sock *sk;
	struct proto *answer_prot;
	unsigned char answer_flags;
	char answer_no_check;
	int err;
	
	/* These variables need to be set without answer structure */
	answer_prot = NULL; // answer->prot;
	answer_no_check = 0; //answer->no_check;
	answer_flags = 0; //answer->flags;
	err = -ENOBUFS;
	sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot);
	if (sk == NULL)
		goto out;
	
	err = 0;
	sk->sk_no_check = answer_no_check;
	if (INET_PROTOSW_REUSE & answer_flags)
		sk->sk_reuse = 1;
	
	//some equal of sock_init_data(sock, sk)
	
	sk->sk_destruct = inet_sock_destruct;
	sk->sk_family = PF_INET;
	sk->sk_protocol = protocol;
	sk->sd_backlog_rcv = sk->sk_prot->backlog_rcv;

	if(sk->sk_prot->init) {
		err = sk->sk_prot->init(sk);
		if(err)
			sk_common_release(sk);
	}
out:
	return err;
}

static long inet_wait_for_connect(struct sock *sk, long timeo)
{
	DEFINE_WAIT(wait);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);

	while((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) {
		release_sock(sk);
		timeo = schedule_timeout(timeo);
		lock_sock(sk);
		if (signal_pending(current) || !timeo)
			break;
		prepare_to_wait(sk->sleep, &wait, TASK_INTERRUPTIBLE);
	}

	finish_wait(sk->sk_sleep, &wait);
	return timeo;
}

/* 
 * Connect to a remote host.
 */
int slashnet_stream_connect(struct file *filp, struct sockaddr *uaddr
		int addr_len, int flags)
{
	struct sock *sk; // = what?
	int err;
	long timeo;

	lock_sock(sk);
	//increment value of /net/tcp/clone ?

	if (uaddr->sa_family == AF_UNSPEC) {
		err = sk->sk_prot->disconnect(sk, flags);
		//print "Closed" in /net/tcp/n/status
		goto out;
	}

	err = -EISCONN;

	if (sk->sk_state != TCP_CLOSE)
		goto out;

	err = sk->sk_prot_connect(sk, uaddr, addr_len);
	if (err < 0)
		goto out;
	
	//print "Time_Wait" in /net/tcp/n/status
	
	timeo = sock_sndtimeo(sk, flags & O_NONBLOCK);

	if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) {
		/* wait for timeout */
		if (!timeo || !inet_wait_for_connect(sk, timeo))
			goto out;
	
	if (sk->sk_state == TCP_CLOSE) {
		err = sock_err(sk) ? : -ECONNABORTED;
		//print "Closed" in /net/tcp/n/status
		if (sk->sk_prot->disconnect(sk, flags)) {
			; 	//print "Pending" in /net/tcp/n/status
		}

		goto out;
	}

	//print "Connected" in /net/tcp/n/status
	err = 0;
out:
	release_sock(sk);
	return err;
}

