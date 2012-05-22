/*
 * libshortcut
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>, Youngjoo Park <yjoo93.park@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software, either express or implied, including but not limited to the implied warranties of merchantability, fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using, modifying or distributing this software or its derivatives.
 *
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <glib.h>
#include <dlog.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <secom_socket.h>
#include <shortcut.h>

#include <sys/socket.h>
#include <sys/ioctl.h>



#define EAPI __attribute__((visibility("default")))
#define LIVEBOX_FLAG	0x0100
#define TYPE_MASK	0x000F


extern int errno;



struct server_cb {
	request_cb_t request_cb;
	void *data;
};



struct client_cb {
	result_cb_t result_cb;
	void *data;
};



static struct info {
	pthread_mutex_t server_mutex;
	int server_fd;
	const char *socket_file;
	struct server_cb server_cb;
	unsigned int seq;
} s_info = {
	.server_mutex = PTHREAD_MUTEX_INITIALIZER,
	.server_fd = -1,
	.socket_file = "/tmp/.shortcut",
	.seq = 0,
};



struct packet {
	struct {
		unsigned int seq;
		enum {
			PACKET_ERR = 0x0,
			PACKET_REQ,
			PACKET_ACK,
			PACKET_MAX = 0xFF, /* MAX */
		} type;

		int payload_size;

		union {
			struct {
				int shortcut_type;
				struct {
					int pkgname;
					int name;
					int exec;
					int icon;
				} field_size;
			} req;

			struct {
				int ret;
			} ack;
		} data;
	} head;

	char payload[];
};



struct connection_state {
	void *data;
	struct packet packet;
	enum {
		BEGIN,
		HEADER,
		PAYLOAD,
		END,
		ERROR,
	} state;
	int length;
	int from_pid;
	char *payload;
};



static inline
gboolean check_reply_service(int conn_fd, struct connection_state *state)
{
	struct client_cb *client_cb = state->data;

	if (client_cb->result_cb) {
		/* If the callback return FAILURE,
		 * remove itself from the callback list */
		client_cb->result_cb(state->packet.head.data.ack.ret,
					state->from_pid, client_cb->data);
	}

	state->state = BEGIN;
	state->length = 0;
	state->from_pid = 0;

	/* NOTE: If we want close the connection, returns FALSE */
	return FALSE;
}



static inline
gboolean do_reply_service(int conn_fd, struct connection_state *state)
{
	int ret;
	struct packet send_packet;

	ret = -ENOSYS;
	if (s_info.server_cb.request_cb) {
		char *pkgname;
		char *name;
		char *exec;
		char *icon;
		double period;

		if (state->packet.head.data.req.field_size.pkgname)
			pkgname = state->payload;
		else
			pkgname = NULL;

		if (state->packet.head.data.req.field_size.name)
			name = state->payload + state->packet.head.data.req.field_size.pkgname;
		else
			name = NULL;

		if (state->packet.head.data.req.field_size.exec) {
			exec = state->payload + state->packet.head.data.req.field_size.pkgname;
			exec += state->packet.head.data.req.field_size.name;
		} else {
			exec = NULL;
		}

		if (state->packet.head.data.req.field_size.icon) {
			icon = state->payload + state->packet.head.data.req.field_size.pkgname;
			icon += state->packet.head.data.req.field_size.name;
			icon += state->packet.head.data.req.field_size.exec;
		} else {
			icon = NULL;
		}

		if (state->packet.head.data.req.shortcut_type & LIVEBOX_FLAG) {
			char *tmp;
			tmp = state->payload + state->packet.head.data.req.field_size.pkgname;
			tmp += state->packet.head.data.req.field_size.name;
			tmp += state->packet.head.data.req.field_size.exec;
			tmp += state->packet.head.data.req.field_size.icon;
			memcpy(&period, tmp, sizeof(period));
		} else {
			period = -1.0f;
		}

		LOGD("Pkgname: [%s] Type: [%x], Name: [%s], Exec: [%s], Icon: [%s], Period: [%lf]\n",
				pkgname,
				state->packet.head.data.req.shortcut_type,
				name,
				exec,
				icon,
				period);

		ret = s_info.server_cb.request_cb(
				pkgname,
				name,
				state->packet.head.data.req.shortcut_type & TYPE_MASK,
				exec,
				icon,
				state->from_pid,
				period,
				s_info.server_cb.data);
	}

	send_packet.head.type = PACKET_ACK;
	send_packet.head.payload_size = 0;
	send_packet.head.seq = state->packet.head.seq;
	send_packet.head.data.ack.ret = ret;

	if (secom_send(conn_fd, (const char*)&send_packet, sizeof(send_packet)) != sizeof(send_packet)) {
		LOGE("Faield to send ack packet\n");
		return FALSE;
	}

	state->state = BEGIN;
	state->length = 0;
	state->from_pid = 0;
	return TRUE;
}



static inline
gboolean filling_payload(int conn_fd, struct connection_state *state)
{
	if (state->length < state->packet.head.payload_size) {
		int ret;
		int check_pid;

		ret = secom_recv(conn_fd,
			state->payload + state->length,
			state->packet.head.payload_size - state->length,
			&check_pid);
		if (ret < 0)
			return FALSE;

		if (state->from_pid != check_pid) {
			LOGD("PID is not matched (%d, expected %d)\n",
						check_pid, state->from_pid);
			return FALSE;
		}

		state->length += ret;
	}

	if (state->length == state->packet.head.payload_size) {
		state->length = 0;
		state->state = END;
	}

	return TRUE;
}



static inline
gboolean deal_error_packet(int conn_fd, struct connection_state *state)
{
	if (state->length < state->packet.head.payload_size) {
		int ret;
		char *buf;
		int check_pid;

		buf = malloc(state->packet.head.payload_size - state->length);
		if (!buf)
			return FALSE;

		ret = secom_recv(conn_fd,
			buf,
			state->packet.head.payload_size - state->length,
			&check_pid);
		if (ret < 0) {
			free(buf);
			return FALSE;
		}

		if (check_pid != state->from_pid)
			LOGD("PID is not matched (%d, expected %d)\n",
						check_pid, state->from_pid);

		state->length += ret;
		free(buf);
	}

	if (state->length < state->packet.head.payload_size)
		return TRUE;

	/* Now, drop this connection */
	return FALSE;
}



static inline
gboolean filling_header(int conn_fd, struct connection_state *state)
{
	if (state->length < sizeof(state->packet)) {
		int ret;
		int check_pid;

		ret = secom_recv(conn_fd,
			((char*)&state->packet) + state->length, 
			sizeof(state->packet) - state->length,
			&check_pid);
		if (ret < 0)
			return FALSE;

		if (state->from_pid == 0)
			state->from_pid = check_pid;

		if (state->from_pid != check_pid) {
			LOGD("PID is not matched (%d, expected %d)\n",
						check_pid, state->from_pid);
			return FALSE;
		}

		state->length += ret;
	}

	if (state->length < sizeof(state->packet))
		return TRUE;

	if (state->packet.head.type == PACKET_ACK) {
		state->length = 0;

		if (state->packet.head.payload_size)
			state->state = ERROR;
		else
			state->state = END;
	} else if (state->packet.head.type == PACKET_REQ) {
		/* Let's take the next part. */
		state->state = PAYLOAD;
		state->length = 0;

		state->payload = calloc(1, state->packet.head.payload_size);
		if (!state->payload) {
			LOGE("Heap: %s\n", strerror(errno));
			return FALSE;
		}
	} else {
		LOGE("Invalid packet type\n");
		return FALSE;
	}

	return TRUE;
}



static
gboolean client_connection_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int conn_fd;
	gboolean ret;
	int read_size;
	struct connection_state *state = data;
	struct client_cb *client_cb = state->data;

	if (!(cond & G_IO_IN)) {
		LOGE("Condition value is unexpected value\n");
		ret = FALSE;
		goto out;
	}

	conn_fd = g_io_channel_unix_get_fd(src);

	if (ioctl(conn_fd, FIONREAD, &read_size) < 0) {
		LOGE("Failed to get q size\n");
		ret = FALSE;
		goto out;
	}

	if (read_size == 0) {
		if (client_cb->result_cb) {
			/* If the callback return FAILURE,
			 * remove itself from the callback list */
			client_cb->result_cb(-ECONNABORTED,
					state->from_pid, client_cb->data);
		}
		ret = FALSE;
		goto out;
	}

	switch (state->state) {
	case BEGIN:
		state->state = HEADER;
	case HEADER:
		ret = filling_header(conn_fd, state);
		break;
	case ERROR:
		ret = deal_error_packet(conn_fd, state);
		break;
	default:
		ret = FALSE;
		break;
	}

	if (state->state == END)
		ret = check_reply_service(conn_fd, state);

out:
	if (ret == FALSE) {
		close(conn_fd);
		free(state->data);
		free(state);
	}

	return ret;
}



static
gboolean connection_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int conn_fd;
	struct connection_state *state = data;
	gboolean ret;
	int read_size;

	if (!(cond & G_IO_IN)) {
		ret = FALSE;
		goto out;
	}

	conn_fd = g_io_channel_unix_get_fd(src);

	if (ioctl(conn_fd, FIONREAD, &read_size) < 0) {
		LOGE("Failed to get q size\n");
		ret = FALSE;
		goto out;
	}

	if (read_size == 0) {
		LOGE("Disconnected\n");
		ret = FALSE;
		goto out;
	}

	switch (state->state) {
	case BEGIN:
		state->state = HEADER;
	case HEADER:
		ret = filling_header(conn_fd, state);
		break;
	case PAYLOAD:
		ret = filling_payload(conn_fd, state);
		break;
	default:
		LOGE("[%s:%d] Invalid state(%x)\n",
					__func__, __LINE__, state->state);
		ret = FALSE;
		break;
	}

	if (state->state == END) {
		ret = do_reply_service(conn_fd, state);
		if (state->payload) {
			free(state->payload);
			state->payload = NULL;
		}

		memset(&state->packet, 0, sizeof(state->packet));
	}

out:
	if (ret == FALSE) {
		secom_put_connection_handle(conn_fd);

		if (state->payload)
			free(state->payload);

		free(state);
	}

	return ret;
}



static
gboolean accept_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int server_fd;
	int connection_fd;
	GIOChannel *gio;
	guint id;
	struct connection_state *state;

	server_fd = g_io_channel_unix_get_fd(src);
	if (server_fd != s_info.server_fd) {
		LOGE("Unknown FD is gotten.\n");
		/* NOTE:
		 * In this case, don't try to do anything. 
		 * This is not recoverble error */
		return FALSE;
	}

	if (!(cond & G_IO_IN)) {
		close(s_info.server_fd);
		s_info.server_fd = -1;
		return FALSE;
	}

	connection_fd = secom_get_connection_handle(server_fd);
	if (connection_fd < 0) {
		/* Error log will be printed from
		 * get_connection_handle function */
		return FALSE;
	}

	if (fcntl(connection_fd, F_SETFD, FD_CLOEXEC) < 0)
		LOGE("Error: %s\n", strerror(errno));

	if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) < 0)
		LOGE("Error: %s\n", strerror(errno));

	gio = g_io_channel_unix_new(connection_fd);
	if (!gio) {
		LOGE("Failed to create a new connection channel\n");
		secom_put_connection_handle(connection_fd);
		return FALSE;
	}

	state = calloc(1, sizeof(*state));
	if (!state) {
		LOGE("Heap: %s\n", strerror(errno));
		secom_put_connection_handle(connection_fd);
		return FALSE;
	}

	state->state = BEGIN;
	id = g_io_add_watch(gio,
			G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
			(GIOFunc)connection_cb, state);
	if (id < 0) {
		GError *err = NULL;
		LOGE("Failed to create g_io watch\n");
		free(state);
		g_io_channel_unref(gio);
		g_io_channel_shutdown(gio, TRUE, &err);
		secom_put_connection_handle(connection_fd);
		return FALSE;
	}

	g_io_channel_unref(gio);
	return TRUE;
}



static inline int init_client(struct client_cb *client_cb,
					const char *packet, int packet_size)
{
	GIOChannel *gio;
	guint id;
	int client_fd;
	struct connection_state *state;

	client_fd = secom_create_client(s_info.socket_file);
	if (client_fd < 0) {
		LOGE("Failed to make the client FD\n");
		return -EFAULT;
	}

	if (fcntl(client_fd, F_SETFD, FD_CLOEXEC) < 0)
		LOGE("Error: %s\n", strerror(errno));

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
		LOGE("Error: %s\n", strerror(errno));

	gio = g_io_channel_unix_new(client_fd);
	if (!gio) {
		close(client_fd);
		return -EFAULT;
	}

	state = calloc(1, sizeof(*state));
	if (!state) {
		LOGE("Error: %s\n", strerror(errno));
		close(client_fd);
		return -ENOMEM;
	}

	state->data = client_cb;

	id = g_io_add_watch(gio,
		G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
		(GIOFunc)client_connection_cb, state);
	if (id < 0) {
		GError *err = NULL;
		LOGE("Failed to create g_io watch\n");
		free(state);
		g_io_channel_unref(gio);
		g_io_channel_shutdown(gio, TRUE, &err);
		close(client_fd);
		return -EFAULT;
	}

	g_io_channel_unref(gio);

	if (secom_send(client_fd, packet, packet_size) != packet_size) {
		GError *err = NULL;
		LOGE("Failed to send all packet "
			"(g_io_channel is not cleared now\n");
		free(state);
		g_io_channel_shutdown(gio, TRUE, &err);
		close(client_fd);
		return -EFAULT;
	}

	return client_fd;
}



static inline
int init_server(void)
{
	GIOChannel *gio;
	guint id;

	if (s_info.server_fd != -1) {
		LOGE("Already initialized\n");
		return 0;
	}

	if (pthread_mutex_lock(&s_info.server_mutex) != 0) {
		LOGE("[%s:%d] Failed to get lock (%s)\n",
					__func__, __LINE__, strerror(errno));
		return -EFAULT;
	}

	unlink(s_info.socket_file);
	s_info.server_fd = secom_create_server(s_info.socket_file);

	if (s_info.server_fd < 0) {
		LOGE("Failed to open a socket (%s)\n", strerror(errno));
		if (pthread_mutex_unlock(&s_info.server_mutex) != 0)
			LOGE("[%s:%d] Failed to do unlock mutex (%s)\n",
					__func__, __LINE__, strerror(errno));
		return -EFAULT;
	}

	if (fcntl(s_info.server_fd, F_SETFD, FD_CLOEXEC) < 0)
		LOGE("Error: %s\n", strerror(errno));

	if (fcntl(s_info.server_fd, F_SETFL, O_NONBLOCK) < 0)
		LOGE("Error: %s\n", strerror(errno));

	gio = g_io_channel_unix_new(s_info.server_fd);
	if (!gio) {
		close(s_info.server_fd);
		s_info.server_fd = -1;
		if (pthread_mutex_unlock(&s_info.server_mutex) != 0)
			LOGE("[%s:%d] Failed to do unlock mutex (%s)\n",
					__func__, __LINE__, strerror(errno));
		return -EFAULT;
	}

	id = g_io_add_watch(gio,
			G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
			(GIOFunc)accept_cb, NULL);
	if (id < 0) {
		GError *err = NULL;
		LOGE("Failed to create g_io watch\n");
		g_io_channel_unref(gio);
		g_io_channel_shutdown(gio, TRUE, &err);
		close(s_info.server_fd);
		s_info.server_fd = -1;
		if (pthread_mutex_unlock(&s_info.server_mutex) != 0)
			LOGE("[%s:%d] Failed to do unlock mutex (%s)\n",
					__func__, __LINE__, strerror(errno));

		return -EFAULT;
	}

	g_io_channel_unref(gio);

	if (pthread_mutex_unlock(&s_info.server_mutex) != 0) {
		GError *err = NULL;
		g_io_channel_shutdown(gio, TRUE, &err);
		LOGE("[%s:%d] Failed to do unlock mutex (%s)\n",
					__func__, __LINE__, strerror(errno));
		/* NOTE:
		 * We couldn't make a lock for this statements.
		 * We already meet the unrecoverble error
		 */
		close(s_info.server_fd);
		s_info.server_fd = -1;
		return -EFAULT;
	}

	return 0;
}



EAPI int shortcut_set_request_cb(request_cb_t request_cb, void *data)
{
	int ret;
	s_info.server_cb.request_cb = request_cb;
	s_info.server_cb.data = data;

	ret = init_server();
	if (ret != 0) {
		LOGE("Failed to initialize the server\n");
	}

	return ret;
}



EAPI int shortcut_add_to_home(const char *pkgname, const char *name, int type, const char *content, const char *icon, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	int pkgname_len;
	int name_len;
	int exec_len;
	int icon_len;
	int packet_size;
	char *payload;
	struct client_cb *client_cb;

	pkgname_len = pkgname ? strlen(pkgname) + 1 : 0;
	name_len = name ? strlen(name) + 1 : 0;
	exec_len = content ? strlen(content) + 1 : 0;
	icon_len = icon ? strlen(icon) + 1 : 0;

	packet_size = sizeof(*packet) + name_len + exec_len + icon_len + pkgname_len + 1;

	packet = malloc(packet_size);
	if (!packet) {
		LOGE("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	packet->head.seq = s_info.seq++;
	packet->head.type = PACKET_REQ;
	packet->head.data.req.shortcut_type = type;
	packet->head.data.req.field_size.pkgname = pkgname_len;
	packet->head.data.req.field_size.name = name_len;
	packet->head.data.req.field_size.exec = exec_len;
	packet->head.data.req.field_size.icon = icon_len;
	packet->head.payload_size = pkgname_len + name_len + exec_len + icon_len + 1;

	payload = packet->payload;

	strncpy(payload, pkgname, pkgname_len);
	payload += pkgname_len;
	strncpy(payload, name, name_len);
	payload += name_len;
	strncpy(payload, content, exec_len);
	payload += exec_len;
	strncpy(payload, icon, icon_len);

	client_cb = malloc(sizeof(*client_cb));
	if (!client_cb) {
		LOGE("Heap: %s\n", strerror(errno));
		free(packet);
		return -ENOMEM;
	}

	client_cb->result_cb = result_cb;
	client_cb->data = data;

	if (init_client(client_cb, (const char*)packet, packet_size) < 0) {
		LOGE("Failed to init client FD\n");
		free(client_cb);
		free(packet);
		return -EFAULT;
	}

	free(packet);
	return 0;
}



EAPI int shortcut_add_to_home_with_period(const char *pkgname, const char *name, int type, const char *content, const char *icon, double period, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	int pkgname_len;
	int name_len;
	int exec_len;
	int icon_len;
	int packet_size;
	char *payload;
	struct client_cb *client_cb;

	pkgname_len = pkgname ? strlen(pkgname) + 1 : 0;
	name_len = name ? strlen(name) + 1 : 0;
	exec_len = content ? strlen(content) + 1 : 0;
	icon_len = icon ? strlen(icon) + 1 : 0;

	packet_size = sizeof(*packet) + name_len + exec_len + icon_len + pkgname_len + sizeof(period) + 1;

	packet = malloc(packet_size);
	if (!packet) {
		LOGE("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	packet->head.seq = s_info.seq++;
	packet->head.type = PACKET_REQ;
	packet->head.data.req.shortcut_type = LIVEBOX_FLAG | type;
	packet->head.data.req.field_size.pkgname = pkgname_len;
	packet->head.data.req.field_size.name = name_len;
	packet->head.data.req.field_size.exec = exec_len;
	packet->head.data.req.field_size.icon = icon_len;
	packet->head.payload_size = sizeof(period) + pkgname_len + name_len + exec_len + icon_len + 1;

	payload = packet->payload;

	strncpy(payload, pkgname, pkgname_len);
	payload += pkgname_len;
	strncpy(payload, name, name_len);
	payload += name_len;
	strncpy(payload, content, exec_len);
	payload += exec_len;
	strncpy(payload, icon, icon_len);
	payload += icon_len;
	memcpy(payload, &period, sizeof(period));

	client_cb = malloc(sizeof(*client_cb));
	if (!client_cb) {
		LOGE("Heap: %s\n", strerror(errno));
		free(packet);
		return -ENOMEM;
	}

	client_cb->result_cb = result_cb;
	client_cb->data = data;

	if (init_client(client_cb, (const char*)packet, packet_size) < 0) {
		LOGE("Failed to init client FD\n");
		free(client_cb);
		free(packet);
		return -EFAULT;
	}

	free(packet);
	return 0;
}



/* End of a file */
