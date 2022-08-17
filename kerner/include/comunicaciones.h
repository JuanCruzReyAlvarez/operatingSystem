#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

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
#include "auxiliares.h"


typedef struct {
    t_log* log;   // que es esto? Y por que no esta inicializado
    int fd;
    char* server_name;
} t_procesar_conexion_args;

op_code recibir_cop_PRUEBA(int socket_cliente);
op_code recibir_cop(int socket_cliente);
bool generar_conexiones_kerner(t_log* logger, int* fd_dispatch,int* fd_memoria, int* fd_interrupt);
uint32_t read_estimation_from_config();
pcb* estado_NEW(t_list* list_instrucciones,uint32_t tamanioProceso,int cliente_socket);
void* nullANew(t_list* list_instrucciones,uint32_t tamanioProceso,int cliente_socket);
void conectarse_a_memoria_cpu();
void handshake_de_alfa();
int server_escuchar(t_log* logger, char* server_name, int server_socket);



//static void procesar_conexion(void* void_args);
/*
float read_estimation_from_config();
int read_grado_multiprogramacion_from_config();
char* read_algoritmo_from_config();
*/
#endif