#include "listenserver.h"

#include <unistd.h>
#include <signal.h>

#include <cstdio>

static bool listen_open = false;
static pid_t child_pid = -1;

bool isListenServerOpen() {
	return listen_open;
}

int startListenServer(uint16_t port) {
	char port_arg[16];
	sprintf(port_arg, "%d", port);

	pid_t pid = fork();
	if (pid < 0) {
		return 1;
	}

	listen_open = true;

	child_pid = pid;

	if (pid) {
		sleep(1);
	} else {
		execl("/home/salvo/Desktop/bomberman/server/bin/Debug/bomberman_server", "bomberman_server", port_arg, (char *)0);
	}

	return 0;
}

int stopListenServer() {
	listen_open = false;
	return kill(child_pid, SIGKILL);
}