#ifndef LOGS_CPU_H
#define LOGS_CPU_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
//#include "shared_utils.h"
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


void cpu_log_inicial(t_log* logger);
void fetch_log(t_log* logger);
void decode_log(t_log* logger);
void cpu_log_handshake(t_log* logger);
void execute_log(t_log*logger);
void fetchOperands_log(t_log* logger);

#endif

