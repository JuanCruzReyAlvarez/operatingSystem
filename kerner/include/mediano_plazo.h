#ifndef MEDIANO_PLAZO_H
#define MEDIANO_PLAZO_H

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
//#include <commons>
#include <string.h>
#include <inttypes.h>
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"

#include "comunicaciones.h"

#include <semaphore.h>
void* suspendedReady_Ready();
void* suspended_blocked_Suspended_ready();

void* enviarAReady(t_list* estadoNew, t_list* estadoSuspendedReady, int procesosHabilitados);
void ejecutarIO();
void* suspended_blocked_Suspended_ready();
void* blocked_Ready();

#endif