/* Speaker and mic levels from wpctl, only re-read when PipeWire reports a change. No shell. */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "../slstatus.h"
#include "../util.h"

/* Bumped whenever audio may have changed. One long-lived pw-mon prints every PipeWire
 * change; clients come and go with each wpctl call, so anything else counts.
 * ponytail: without pw-mon (pipewire-bin) this bumps every tick, i.e. plain polling. */
static unsigned int
audiogen(void)
{
	static unsigned int gen = 1;
	static int fd = -1;
	static pid_t pid;
	static char line[256];
	static size_t n;
	char buf[4096];
	ssize_t r, i;
	int p[2];

	if (fd < 0) {
		gen++;
		if (pipe(p) < 0)
			return gen;
		if ((pid = fork()) == 0) {
			prctl(PR_SET_PDEATHSIG, SIGTERM);
			dup2(p[1], STDOUT_FILENO);
			close(p[0]);
			close(p[1]);
			execlp("pw-mon", "pw-mon", "-N", (char *)NULL);
			_exit(127);
		}
		close(p[1]);
		if (pid < 0) {
			close(p[0]);
			return gen;
		}
		fd = p[0];
		fcntl(fd, F_SETFL, O_NONBLOCK);
		n = 0;
	}
	while ((r = read(fd, buf, sizeof(buf))) > 0) {
		for (i = 0; i < r; i++) {
			if (buf[i] != '\n') {
				if (n < sizeof(line) - 1)
					line[n++] = buf[i];
				continue;
			}
			line[n] = '\0';
			n = 0;
			if (strstr(line, "type: PipeWire:Interface:") && !strstr(line, ":Client"))
				gen++;
		}
	}
	if (r == 0 || (errno != EAGAIN && errno != EINTR)) {
		close(fd);
		fd = -1;
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
	}
	return gen;
}

static int
wpvol(const char *target, double *v, int *muted)
{
	int fd[2], st;
	char line[128];
	ssize_t n = 0, r;
	pid_t pid;

	if (pipe(fd) < 0)
		return -1;
	pid = fork();
	if (pid < 0) {
		close(fd[0]);
		close(fd[1]);
		return -1;
	}
	if (pid == 0) {
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execlp("wpctl", "wpctl", "get-volume", target, (char *)NULL);
		_exit(127);
	}
	close(fd[1]);
	while (n < (ssize_t)sizeof(line) - 1 &&
	       (r = read(fd[0], line + n, sizeof(line) - 1 - n)) > 0)
		n += r;
	close(fd[0]);
	waitpid(pid, &st, 0);
	line[n] = '\0';
	if (sscanf(line, "Volume: %lf", v) != 1)
		return -1;
	*muted = strstr(line, "MUTED") != NULL;
	return 0;
}

/* a failed read leaves seen behind, so it is retried next tick */
const char *
pwsink(const char *unused)
{
	static char s[32];
	static unsigned int seen;
	static int ok;
	unsigned int g = audiogen();
	double v;
	int muted;

	(void)unused;
	if (g != seen) {
		ok = wpvol("@DEFAULT_AUDIO_SINK@", &v, &muted) == 0;
		if (ok) {
			seen = g;
			if (muted)
				snprintf(s, sizeof(s), "\uf026 vol mute");
			else
				snprintf(s, sizeof(s), "\uf028 vol %d%%", (int)(v * 100.0 + 0.5));
		}
	}
	return ok ? s : NULL;
}

const char *
pwsource(const char *unused)
{
	static const char *s;
	static unsigned int seen;
	unsigned int g = audiogen();
	double v;
	int muted;

	(void)unused;
	if (g != seen) {
		s = NULL;
		if (wpvol("@DEFAULT_AUDIO_SOURCE@", &v, &muted) == 0) {
			seen = g;
			s = muted ? "\uf131 mic mute" : "\uf130 mic unmute";
		}
	}
	return s;
}
