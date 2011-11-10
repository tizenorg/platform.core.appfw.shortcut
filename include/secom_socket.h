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
