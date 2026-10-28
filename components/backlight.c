/* Laptop panel brightness from sysfs. Argument is the backlight directory. */
#include <stdio.h>

#include "../slstatus.h"
#include "../util.h"

const char *
backlight(const char *dir)
{
	int cur, max;
	char path[256];

	if (!dir)
		return NULL;
	if (esnprintf(path, sizeof(path), "%s/brightness", dir) < 0)
		return NULL;
	if (pscanf(path, "%d", &cur) != 1)
		return NULL;
	if (esnprintf(path, sizeof(path), "%s/max_brightness", dir) < 0)
		return NULL;
	if (pscanf(path, "%d", &max) != 1 || max <= 0)
		return NULL;
	return bprintf("%d%%", cur * 100 / max);
}
