#ifndef AUXILIARES_H
#define AUXILIARES_H

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
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"
#include <semaphore.h>

sem_t sem_ready;
sem_t sem_exec;
sem_t sem_blocked;
sem_t sem_new;
sem_t sem_exit;
sem_t sem_suspended_ready;
sem_t sem_suspended_blocked;
sem_t sem_procesosEnMemoria;
sem_t mutex_io;
sem_t quieroIrAExec;
sem_t procesosBlockedReady;
sem_t procesosSuspBlockedSuspReady;
sem_t listaBloqueadosAlDia;
sem_t procesosEnIO;
sem_t queHayaLugarEnMemoria;
sem_t procesosEnReady;
sem_t llegoPCBExec;
sem_t mutex_cpu_vacia;
sem_t mutex_recv_dispatch;
sem_t quieroIrAMemoria;
sem_t mutex_mandarAReady;
sem_t meQuieroIr;
sem_t procesosSuspReadyReady;
sem_t procesosNewReady;
sem_t unoEnIOALaVez;
sem_t mutex_atenderBloqueo;


t_list* estadoReady;
pcb* estadoExec;
t_list* estadoBlocked;
t_list* estadoNew;
t_list* estadoExit;
t_list* estadoSuspendedReady;
t_list* estadoSuspendedBlocked;


typedef struct {
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
    char* estimacion_inicial;
    char* alfa;
    char* grado_multiprogramacion;
    char* tiempo_maximo_bloqueado;
} datosConfig;

typedef struct {
    pcb* pcbBloqueada;
    uint32_t tiempoDeBloqueo;
} pcbIO;

bool esperandoDesalojo;
bool cpuOcupada;

pcb* pcbRecibidaDispatch;
uint32_t intRecibidoDispatch;
uint32_t idRecibidoDispatch;


uint32_t procesosEnMemoria;
datosConfig* config;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_a_consola;

t_log* logger;


// FUNCIONES

void* init_semaforos();
void* crear_listas();
void config_get_string_values(t_config* nuevo_config);
void* agregar_instruccion_lista_de_instrucciones(op_code cop,uint32_t param1,uint32_t param2,t_list* list,int cant_instrucciones);
void* calcularSRT();
void decidirProximoEstado(pcb* pcbRecibida);
void ejecutarIO();
void chequearSiTengoQueSuspenderlo(pcbIO* pcbIOBloqueada);
void mandarAReady(pcb* pcbRecibida);
void atenderBloqueo();
void conexionesPorDispatch();
void interrumpirCPU();
void peleaEntreNewSuspReady();
void mandarAMemoria();
void* calcular_FIFO();
void* agregarAMemoria(pcb* pcbAEnviarAMemoria);
void* sacarPCBDeMemoria(pcb* pcbASacar);
void enviarMensajeDeFinalizacion(pcb* pcbFinalizadaCpu);
void levantarHilos();
pcbIO* inicializarPCBIO();
t_list* copiarListaInstrucciones(t_list* origen);
void copiarPCB(pcb* destino, pcb* origen);


//void* init_semaphores(sem_t sem_ready,sem_t sem_exec,sem_t sem_blocked,sem_t sem_new,sem_t sem_exit,sem_t sem_suspended_ready,sem_t sem_suspended_blocked);
//void* crear_listas(t_list* estadoReady,t_list* estadoExec, t_list* estadoBlocked, t_list* estadoNew, t_list* estadoExit, t_list* estadoSuspendedReady,t_list*  estadoSuspendedBlocked);
//bool generar_conexiones(t_log* logger, int* fd_kernel); -> POR QUE ESTA ACA? PREGUNTAR A STEPHY
//uint32_t read_estimation_from_config();
//int read_grado_multiprogramacion_from_config();
//char* read_algoritmo_from_config();
//void* enviarAReady(t_list* estadoNew, t_list* estadoSuspendedReady, int procesosHabilitados);
//uint32_t getProcesosHabilitados();

#endif