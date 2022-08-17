#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/txt.h>
#include <commons/config.h>
#include <commons/process.h>
#include <commons/memory.h>
#include <string.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <semaphore.h>



typedef enum {
    NO_OP,
    IO,
    WRITE,
    COPY,
    READ,
    EXIT
} op_code;

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    SUSPENDED_BLOCKED,
    SUSPENDED_READY,
    FIN
} estado;

typedef enum {
    FIFO,
    LRU
} algoritmoTLB;

typedef struct {
    char* id_INSTRUCCION;
    uint32_t param[2];
} instruccion;

typedef struct {
    uint32_t id;
    uint32_t tamanio_bytes;
    t_list* listaInstrucciones;
    estado estado_proceso;
    uint32_t program_counter;
    uint32_t tabla_paginas;
    uint32_t estimacion_rafaga;
} pcb;

char* mi_funcion_compartida();
int cantidadParametros (char*);
op_code generarCode(char *inst);
char* generarString(op_code inst);
char* generar_string_de_estado(estado estado_proceso);
float rafaga_restante(pcb* pcb_rafaga);
uint32_t id_a_indice(uint32_t id,t_list* lista);
algoritmoTLB generarEnumAlgoritmo(char *algoritmo);

//int fd_cpu_a_kernel;
int fd_kerner;
int fd_memoria;


#endif