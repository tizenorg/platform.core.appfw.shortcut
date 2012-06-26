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
