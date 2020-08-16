#ifndef __LISTENSERVER_H__
#define __LISTENSERVER_H__

#include <stdint.h>

bool isListenServerOpen();

int startListenServer(uint16_t port);

int stopListenServer();

#endif