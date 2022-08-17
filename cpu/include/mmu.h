
#ifndef MMU_H
#define MMU_H

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
#include "tlb.h"

int fd_cpu_a_memoria;
t_log* logger_mmu;

void* requestTablaPagina2doNivel(pcb* pcb,uint32_t tablaPagina_1er_nivel, uint32_t entrada_tablaPagina_1er_nivel, uint32_t* tablaPagina_2do_nivel);
void* requestMarco(pcb* pcb, uint32_t tablaPagina_2do_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t* marco);
void* getNumeroPagina(uint32_t direccion_logica, uint32_t* numero_pagina);
void* getEntradaTablaPagina1erNivel(uint32_t numero_pagina,uint32_t cant_entradas_por_tabla, uint32_t* entrada_tablaPagina_1er_nivel);
void* getEntradaTablaPagina2doNivel(uint32_t cant_entradas_por_tabla, uint32_t numero_pagina,uint32_t* entrada_tabla_2do_nivel);
void* getDesplazamiento(uint32_t direccion_logica,uint32_t numero_pagina, uint32_t* desplazamiento);
void* getDireccionFisica(uint32_t marco, uint32_t* direccion_fisica,uint32_t direccion_logica);
void* startMemoryManagmentUnit(pcb* pcb, uint32_t direccion_logica, uint32_t tablaPagina_1er_nivel, uint32_t cant_entradas_por_tabla,uint32_t* marco);


#endif