#ifndef ALGORITMOS_H
#define ALGORITMOS_H

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
#include <commons/collections/dictionary.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"
#include "tlb.h"
#include <math.h> // ojardo con este


void* replaceByFifo(uint32_t* marco, char* pagina);
void* replaceByLru(uint32_t* marco, char* pagina);
void* excecLRU();
void* delete_all_char(char* elementToDelete,t_list * list);
bool  todosCharsIguales(void *elementoDeLaLista);
bool _is_this_pagina_LRU(char* paginaLista);
bool _is_this_pagina_FIFO(char* paginaLista);

#endif