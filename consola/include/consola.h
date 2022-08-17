#ifndef CONSOLA_H
#define CONSOLA_H

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
#include "../../shared/include/sockets.h"
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
/*
typedef struct {
    char* id_INSTRUCCION;
    uint32_t param[2];
} instruccion;
*/

int fd_consola_a_kernel;

void agregarNuevaInstruccionALaLista(char* linea, t_list* listaInstrucciones);
bool generar_conexiones(t_log* logger, int* fd_kernel);
t_list* agregarInstrucciones(t_list* listaInstrucciones, FILE *archivoInstrucciones);

#endif