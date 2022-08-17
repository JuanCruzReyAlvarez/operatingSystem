
#ifndef CICLO_CPU_H
#define CICLO_CPU_H

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
#include "tlb.h"

int server_escuchar(t_log* logger, char* server_name, int server_socket);
void iniciar_CPU( pcb* pcb, t_log* logger );
void* comenzarEtapaDeEjecucion(pcb* pcb_a, t_log* logger );
void* check_interrupt(pcb* pcb_a);
t_log* crear_logger_cpu();
void* fetch(instruccion** instruccion,pcb* pcb);
void* decode(instruccion* instruccion_completa_actual,op_code* instruccion,uint32_t parametros[2]); // esto no esta mal?? [2]?
void* fetch_operands(pcb* pcb, uint32_t parametros[2], uint32_t* valorOrigen);
void* execute(op_code instruccion,uint32_t parametros[2],pcb* pcb,time_t t_inicial, uint32_t valor );
void estimacion_log(t_log* logger);
void* actualizar_program_counter_pcb(pcb* pcb);
instruccion*  obtener_instruccion_a_ejecutar(pcb* pcb);
void* ejecutarNO_OP(pcb* pcb, time_t t_inicial);
void* ejecutarIO(pcb* pcb, uint32_t parametros[2],time_t t_inicial);
void* ejecutarEXIT(pcb* pcb);
void* actualizar_tiempo_estimado(pcb* pcb, time_t t_inicial, time_t t_final);
float calcular_estimacion_rafaga(float cpu_usado, float alfa, float est_ant);
void* actualizar_tiempo_estimado_sin_forumula(pcb* pcb, time_t t_inicial, time_t t_final);
//void* handshake(float tamanio_de_la_pagina,int tablaDePagina,t_log* logger);
char* puertoDispatch();
float read_retardo_op_from_config();
float read_alfa_from_config();
//void printValor(uint32_t valor);
uint32_t read_tiempo_de_bloqueo_from_config();
void* actualizar_tiempo_estimado_sin_formula(pcb* pcb, time_t t_inicial, time_t t_final);
void start_again(pcb* pcb);
void* runReadConnection(pcb* pcb, uint32_t direcion_fisica, uint32_t* valor);
int fd_cpu_a_kernel;
t_log* logger;
int fd_dispatch;
int fd_interrupt;     // por lo menos hasta que el otro deje de ser 0
float alfa_recibido;
time_t t_inicial;
void* ejecutarREAD(pcb* pcb, uint32_t parametros[2], time_t t_inicial);
void* ejecutarWRITE(pcb* pcb, uint32_t parametros[2], time_t t_inicial);
void* ejecutarCOPY(pcb* pcb, uint32_t parametros[2], time_t t_inicial,uint32_t valor);
void* runWriteConnection(pcb* pcb, uint32_t  direccionFisica, uint32_t valor);
void resetTLB();
//int cliente_socket;

#endif