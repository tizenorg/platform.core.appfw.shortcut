
typedef int (*request_cb_t)(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, int pid, void *data);
typedef int (*result_cb_t)(int ret, int pid, void *data);

enum {
	SHORTCUT_PACKAGE = 0x0,
	SHORTCUT_DATA = 0x01,
	SHORTCUT_FILE = 0x02,
};

/*!
 * \brief
 * \param[in] request_cb 
 * \param[in] data
 * \return void
 */
extern void shortcut_set_request_cb(request_cb_t request_cb, void *data);

/*!
 * \brief
 * \param[in] pkgname
 * \param[in] name
 * \param[in] type
 * \param[in] content_info
 * \param[in] icon
 * \param[in] result_cb
 * \param[in] data
 * \return int
 */
extern int shortcut_add_to_home(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, result_cb_t result_cb, void *data);

/* End of a file */
