/*
 * [libslp-shortcut]
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>,
 *          Youngjoo Park <yjoo93.park@samsung.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>

#include <secom_socket.h>
#include <dlog.h>



extern int errno;



inline static
int create_socket(const char *peer, struct sockaddr_un *addr)
{
	int len;
	int handle;

	len = sizeof(struct sockaddr_un);
	bzero(addr, len);

	if (strlen(peer) >= sizeof(addr->sun_path)) {
		LOGE("peer %s is too long to remember it\\n", peer);
		return -1;
	}

	// We can believe this has no prob, because 
	// we already check the size of add.rsun_path
	strcpy(addr->sun_path, peer);
	addr->sun_family = AF_UNIX;

	handle = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (handle < 0) {
		LOGE("Failed to create a socket %s\n", strerror(errno));
		return -1;
	}

	return handle;
}



int secom_create_client(const char *peer)
{
	struct sockaddr_un addr;
	int handle;
	int state;
	int on = 1;

	handle = create_socket(peer, &addr);
	if (handle < 0)
		return handle;

	state = connect(handle, (struct sockaddr*)&addr, sizeof(addr));
	if (state < 0) {
		LOGE("Failed to connect to server [%s] %s\n", peer, strerror(errno));
		if (close(handle) < 0)
			LOGE("Failed to close a handle\n");

		return -1;
	}

	if (setsockopt(handle, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0)
		LOGE("Failed to change sock opt : %s\n", strerror(errno));

	return handle;
}



int secom_create_server(const char *peer)
{
	int handle;
	int state;
	struct sockaddr_un addr;

	handle = create_socket(peer, &addr);
	if (handle < 0) return handle;

	state = bind(handle, &addr, sizeof(addr));
	if (state < 0) {
		LOGE("Failed to bind a socket %s\n", strerror(errno));
		if (close(handle) < 0) {
			LOGE("Failed to close a handle\n");
		}
		return -1;
	}

	state = listen(handle, 5);
	if (state < 0) {
		LOGE("Failed to listen a socket %s\n", strerror(errno));
		if (close(handle) < 0) {
			LOGE("Failed to close a handle\n");
		}
		return -1;
	}

	if (chmod(peer, 0666) < 0) {
		LOGE("Failed to change the permission of a socket (%s)\n", strerror(errno));
	}

	return handle;
}



int secom_get_connection_handle(int server_handle)
{
	struct sockaddr_un addr;
	int handle;
	int on = 1;
	socklen_t size = sizeof(addr);

	handle = accept(server_handle, (struct sockaddr*)&addr, &size);
	if (handle < 0) {
		LOGE("Failed to accept a new client %s\n", strerror(errno));
		return -1;
	}

	if (setsockopt(handle, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
		LOGE("Failed to change sock opt : %s\n", strerror(errno));
	}

	return handle;
}



int secom_put_connection_handle(int conn_handle)
{
	if (close(conn_handle) < 0) {
		LOGE("Failed to close a handle\n");
		return -1;
	}
	return 0;
}



int secom_send(int handle, const char *buffer, int size)
{
	struct msghdr msg;
	struct iovec iov;
	int ret;

	memset(&msg, 0, sizeof(msg));
	iov.iov_base = (char*)buffer;
	iov.iov_len = size;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	ret = sendmsg(handle, &msg, 0);
	if (ret < 0) {
		LOGE("Failed to send message [%s]\n", strerror(errno));
		return -1;
	}
	LOGD("Send done: %d\n", ret);

	return iov.iov_len;
}



int secom_recv(int handle, char *buffer, int size, int *sender_pid)
{
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;
	int _pid;
	int ret;
	char control[1024];

	if (!sender_pid) sender_pid = &_pid;
	*sender_pid = -1;

	memset(&msg, 0, sizeof(msg));
	iov.iov_base = buffer;
	iov.iov_len = size;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	ret = recvmsg(handle, &msg, 0);
	if (ret < 0) {
		LOGE("Failed to recvmsg [%s] (%d)\n", strerror(errno), ret);
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	while (cmsg) {
		if (cmsg->cmsg_level == SOL_SOCKET
			&& cmsg->cmsg_type == SCM_CREDENTIALS)
		{
			struct ucred *cred;
			cred = (struct ucred*)CMSG_DATA(cmsg);
			*sender_pid = cred->pid;
		}

		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}

	return iov.iov_len;
}



int secom_destroy(int handle)
{
	if (close(handle) < 0) {
		LOGE("Failed to close a handle\n");
		return -1;
	}
	return 0;
}



#undef _GNU_SOURCE
// End of a file
