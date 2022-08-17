
#ifndef TLB_H
#define TLB_H

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
#include "logs_cpu.h"
#include <math.h> // ojardo con este
#include "mmu.h"
#include "algoritmos.h"
#include <math.h> // ojardo con este


void* runTranslationLookasideBuffer(pcb* pcb, uint32_t direccion_logica,uint32_t tablaPagina_1er_nivel,uint32_t cant_entradas_por_tabla,uint32_t *marco);
void* executeTablaTLB(t_dictionary* tablaTLB,uint32_t direccion_logica,uint32_t* marco,uint32_t tamanio_pagina,uint32_t cant_entradas_por_tabla,
uint32_t tablaPagina_1er_nivel);
void* assessDireccionLogica(uint32_t* marco, char* pagina_key);
int readEntradasTLBFromConfig();
char* readReemplazoTLBFromConfig();
int tlbWasUsed();
void* executeRowTLB(pcb* pcb, uint32_t direccion_logica,uint32_t* marco,uint32_t cant_entradas_por_tabla,
uint32_t tablaPagina_1er_nivel, char* pagina_key);
void config_get_string_values(t_config* nuevo_config);

t_dictionary* rowTLB;
t_list* timeStateTablaDirecciones;
t_list * listLRU;
t_list * listSeguridadTLB;

char* keyToDelete;
char* primer_valor_lista_LRU;
t_log* logger_tlb;
typedef struct{
    uint32_t nro_marco;
} marcoTLB;

typedef struct {
    char* algoritmo_reemplazo;
    uint32_t cant_entradas_tlb;
} datosConfig;

uint32_t tamanio_de_la_pagina;
uint32_t cantidadEntradastablaDePagina;
datosConfig* config;

#endif