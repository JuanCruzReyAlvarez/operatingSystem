#ifndef PROTOCOLO_H
#define PROTOCOLO_H

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
#include <string.h>
#include <inttypes.h>
#include "shared_utils.h"
#include <unistd.h>
/*
typedef enum {
    NO_OP,
    IO,
    WRITE,
    COPY,
    READ,
    EXIT
} op_code;
*/

/*
bool send_EXIT(int conexion);
bool send_instruccion_1(int conexion, op_code cop, uint32_t param);
bool send_instruccion_2(int conexion, op_code cop, uint32_t param1, uint32_t param2);
*/
bool send_lista_instrucciones(int conexion, t_list* lista, uint32_t tamanioProceso);


bool recv_un_parametro(int conexion, uint32_t* param);
bool recv_dos_parametros(int conexion, uint32_t* param1, uint32_t* param2);
bool recv_cero_parametros(int conexion);
bool send_PCB (int conexion, pcb* pcbASerializar);
//static void* serializar_PCB (pcb* pcbASerializar, size_t* tamanio_stream);
bool recv_PCB (int conexion, pcb* pcbRecibida);
//static void deserializar_PCB(void* payload, pcb* pcbRecibida); // aber
uint32_t recv_PCB_mas_int (int conexion, pcb* pcbRecibida);
bool send_PCB_mas_int (int conexion, pcb* pcbASerializar, uint32_t elInt);
bool send_mensaje_memoria(int fd, char* mensaje, uint32_t referencia);
bool recv_mensaje_memoria(int fd, char** mensaje, uint32_t* referencia);
bool send_dos_uint(int fd, uint32_t numero1, uint32_t numero2);
bool recv_dos_uint(int fd, uint32_t* numero1, uint32_t* numero2);

bool recv_mensaje_consola(int fd, char** mensaje);
bool send_mensaje_consola(int fd, char* mensaje);

bool recv_mensaje_interrupcion(int fd, char** mensaje);
bool send_mensaje_interrupcion(int fd, char* mensaje);

bool recv__char_y_dos_int(int fd, char** mensaje, uint32_t* referencia1,uint32_t* referencia2);
bool send__char_y_dos_int(int fd, char* mensaje, uint32_t referencia1, uint32_t referencia2);

bool recv_char_y_tres_int(int fd, char** mensaje, uint32_t* referencia1,uint32_t* referencia2,uint32_t* referencia3);
bool send_char_y_tres_int(int fd, char* mensaje, uint32_t referencia1, uint32_t referencia2,uint32_t referencia3);


#endif