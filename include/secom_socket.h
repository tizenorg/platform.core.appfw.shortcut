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

/*
 * Create client connection
 */
extern int secom_create_client(const char *peer);

/*
 * Create server connection
 */
extern int secom_create_server(const char *peer);

/*
 * Get the raw handle to use it for non-blocking mode.
 */
extern int secom_get_connection_handle(int server_handle);
extern int secom_put_connection_handle(int conn_handle);

/*
 * Send data to the connected peer.
 */
extern int secom_send(int conn, const char *buffer, int size);

/*
 * Recv data from the connected peer. and its PID value
 */
extern int secom_recv(int conn, char *buffer, int size, int *sender_pid);

/*
 * Destroy a connection
 */
extern int secom_destroy(int conn);

/* End of a file */
