#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/txt.h>
#include <commons/config.h>
#include <commons/process.h>
#include <commons/memory.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"
#include "auxiliares.h"

//static void procesar_conexion(void* void_args);
int server_escuchar(t_log* logger, char* server_name, int server_socket);
void conectarseAKernelMemoria();

t_log* logger;

#endif