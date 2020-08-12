/* sqUnixEvdevDebug.c -- libevdev mouse & keyboard debug
 *
 * Author: Ken.Dickey@whidbey.com
 * Last Updated: August 2020
 */

/* ============= DEBUG_EVENTS ============== */

/**
 * Look up an entry in the query_modes table by its textual name.
 *
 * @param mode The name of the entry to be found.
 *
 * @return The requested query_mode, or NULL if it could not be found.
 */
static const struct query_mode *find_query_mode_by_name(const char *name)
{
	int i;
	for (i = 0; i < sizeof(query_modes) / sizeof(*query_modes); i++) {
		const struct query_mode *mode = &query_modes[i];
		if (strcmp(mode->name, name) == 0)
			return mode;
	}
	return NULL;
}

/**
 * Look up an entry in the query_modes table by value.
 *
 * @param event_type The value of the entry to be found.
 *
 * @return The requested query_mode, or NULL if it could not be found.
 */
static const struct query_mode *find_query_mode_by_value(int event_type)
{
	int i;
	for (i = 0; i < sizeof(query_modes) / sizeof(*query_modes); i++) {
		const struct query_mode *mode = &query_modes[i];
		if (mode->event_type == event_type)
			return mode;
	}
	return NULL;
}

/**
 * Find a query_mode based on a string identifier. The string can either
 * be a numerical value (e.g. "5") or the name of the event type in question
 * (e.g. "EV_SW").
 *
 * @param query_mode The mode to search for
 *
 * @return The requested code's numerical value, or negative on error.
 */
static const struct query_mode *find_query_mode(const char *query_mode)
{
	if (isdigit(query_mode[0])) {
		unsigned long val;
		errno = 0;
		val = strtoul(query_mode, NULL, 0);
		if (errno)
			return NULL;
		return find_query_mode_by_value(val);
	} else {
		return find_query_mode_by_name(query_mode);
	}
}


/**
 * Convert a string to a specific key/snd/led/sw code. The string can either
 * be the name of the key in question (e.g. "SW_DOCK") or the numerical
 * value, either as decimal (e.g. "5") or as hex (e.g. "0x5").
 *
 * @param mode The mode being queried (key, snd, led, sw)
 * @param kstr The string to parse and convert
 *
 * @return The requested code's numerical value, or negative on error.
 */
static int get_keycode(const struct query_mode *query_mode, const char *kstr)
{
	if (isdigit(kstr[0])) {
		unsigned long val;
		errno = 0;
		val = strtoul(kstr, NULL, 0);
		if (errno) {
			fprintf(stderr, "Could not interpret value %s\n", kstr);
			return -1;
		}
		return (int) val;
	} else {
		const char * const *keynames = names[query_mode->event_type];
		int i;

		for (i = 0; i < query_mode->max; i++) {
			const char *name = keynames[i];
			if (name && strcmp(name, kstr) == 0)
				return i;
		}

		return -1;
	}
}

/**
 * Filter for the AutoDevProbe scandir on /dev/input.
 *
 * @param dir The current directory entry provided by scandir.
 *
 * @return Non-zero if the given directory entry starts with "event", or zero
 * otherwise.
 */
static int is_event_device(const struct dirent *dir) {
	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

/**
 * Scans all /dev/input/event*, display them and ask the user which one to
 * open.
 *
 * @return The event device file name of the device file selected. This
 * string is allocated and must be freed by the caller.
 */
static char* scan_devices(void)
{
	struct dirent **namelist;
	int i, ndev, devnum, match;
	char *filename;
	int max_device = 0;

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
	if (ndev <= 0)
		return NULL;

	fprintf(stderr, "Available devices:\n");

	for (i = 0; i < ndev; i++)
	{
		char fname[PATH_MAX];
		int fd = -1;
		char name[256] = "???";

		snprintf(fname, sizeof(fname),
			 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);

		fprintf(stderr, "%s:	%s\n", fname, name);
		close(fd);

		match = sscanf(namelist[i]->d_name, "event%d", &devnum);
		if (match >= 1 && devnum > max_device)
			max_device = devnum;

		free(namelist[i]);
	}

	fprintf(stderr, "Select the device event number [0-%d]: ", max_device);

	match = scanf("%d", &devnum);
	if (match < 1 || devnum > max_device || devnum < 0)
		return NULL;

	if (asprintf(&filename, "%s/%s%d",
		     DEV_INPUT_EVENT, EVENT_DEV_NAME,
		     devnum) < 0)
		return NULL;

	return filename;
}

static void print_absdata(int fd, int axis)
{
	int abs[6] = {0};
	int k;

	ioctl(fd, EVIOCGABS(axis), abs);
	for (k = 0; k < 6; k++)
		if ((k < 3) || abs[k])
			printf("      %s %6d\n", absval[k], abs[k]);
}

static void print_repdata(int fd)
{
	int i;
	unsigned int rep[2];

	ioctl(fd, EVIOCGREP, rep);

	for (i = 0; i <= REP_MAX; i++) {
		printf("    Repeat code %d (%s)\n", i, names[EV_REP] ? (names[EV_REP][i] ? names[EV_REP][i] : "?") : "?");
		printf("      Value %6d\n", rep[i]);
	}

}

static inline const char* typename(unsigned int type)
{
	return (type <= EV_MAX && events[type]) ? events[type] : "?";
}

static inline const char* codename(unsigned int type, unsigned int code)
{
	return (type <= EV_MAX && code <= maxval[type] && names[type] && names[type][code]) ? names[type][code] : "?";
}

#ifdef INPUT_PROP_SEMI_MT
static inline const char* propname(unsigned int prop)
{
	return (prop <= INPUT_PROP_MAX && props[prop]) ? props[prop] : "?";
}
#endif

static int get_state(int fd, unsigned int type, unsigned long *array, size_t size)
{
	int rc;

	switch(type) {
	case EV_LED:
		rc = ioctl(fd, EVIOCGLED(size), array);
		break;
	case EV_SND:
		rc = ioctl(fd, EVIOCGSND(size), array);
		break;
	case EV_SW:
		rc = ioctl(fd, EVIOCGSW(size), array);
		break;
	case EV_KEY:
		/* intentionally not printing the value for EV_KEY, let the
		 * repeat handle this */
	default:
		return 1;
	}
	if (rc == -1)
		return 1;

	return 0;
}

/**
 * Print static device information (no events). This information includes
 * version numbers, device name and all bits supported by this device.
 *
 * @param fd The file descriptor to the device.
 * @return 0 on success or 1 otherwise.
 */
static int print_device_info(int fd)
{
	unsigned int type, code;
	int version;
	unsigned short id[4];
	char name[256] = "Unknown";
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
	unsigned long state[KEY_CNT] = {0};
#ifdef INPUT_PROP_SEMI_MT
	unsigned int prop;
	unsigned long propbits[INPUT_PROP_MAX];
#endif
	int stateval;
	int have_state;

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		perror("evtest: can't get version");
		return 1;
	}

	printf("Input driver version is %d.%d.%d\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);

	ioctl(fd, EVIOCGID, id);
	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Input device name: \"%s\"\n", name);

	memset(bit, 0, sizeof(bit));
	ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
	printf("Supported events:\n");

	for (type = 0; type < EV_MAX; type++) {
		if (test_bit(type, bit[0]) && type != EV_REP) {
			have_state = (get_state(fd, type, state, sizeof(state)) == 0);

			printf("  Event type %d (%s)\n", type, typename(type));
			if (type == EV_SYN) continue;
			ioctl(fd, EVIOCGBIT(type, KEY_MAX), bit[type]);
			for (code = 0; code < KEY_MAX; code++)
				if (test_bit(code, bit[type])) {
					if (have_state) {
						stateval = test_bit(code, state);
						printf("    Event code %d (%s) state %d\n",
						       code, codename(type, code), stateval);
					} else {
						printf("    Event code %d (%s)\n", code, codename(type, code));
					}
					if (type == EV_ABS)
						print_absdata(fd, code);
				}
		}
	}

	if (test_bit(EV_REP, bit[0])) {
		printf("Key repeat handling:\n");
		printf("  Repeat type %d (%s)\n", EV_REP, events[EV_REP] ?  events[EV_REP] : "?");
		print_repdata(fd);
	}
#ifdef INPUT_PROP_SEMI_MT
	memset(propbits, 0, sizeof(propbits));
	ioctl(fd, EVIOCGPROP(sizeof(propbits)), propbits);
	printf("Properties:\n");
	for (prop = 0; prop < INPUT_PROP_MAX; prop++) {
		if (test_bit(prop, propbits))
			printf("  Property type %d (%s)\n", prop, propname(prop));
	}
#endif

	return 0;
}


#endif
