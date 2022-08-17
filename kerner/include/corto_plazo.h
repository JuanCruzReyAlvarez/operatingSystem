#ifndef CORTO_PLAZO_H
#define CORTO_PLAZO_H

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


void* blocked_Ready();
void planificarCortoPlazo();

#endif