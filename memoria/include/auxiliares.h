#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdio.h>
#include <math.h>
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
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../shared/include/shared_utils.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"

typedef struct {
    char* puerto_escucha;
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t entradas_por_tabla;
    uint32_t retardo_memoria;
    char* algoritmo_reemplazo;
    uint32_t marcos_por_proceso;
    uint32_t retardo_swap;
    char* path_swap;
} datosConfig;

typedef struct {
    uint32_t marco;
    bool presencia;
    bool uso;
    bool modificado;
} datosPagina;

typedef struct {
    uint32_t pidProceso; 
    uint32_t nroTablaPrimerNivel;
    uint32_t nroTablaSegundoNivel;
    uint32_t entradaPrimerNivel;
    uint32_t entradaSegundoNivel;
} datosProcesoCPU;

typedef struct {
    uint32_t pid;
    bool libre;
} valueTablaMarcos;

typedef struct {
    datosPagina* dataPagina;
    uint32_t nroPagina;
} datosPagNumerada;

typedef struct {
    uint32_t nroTablaNivelDos;
} valueTablaPrimerNivel;

datosProcesoCPU* tablasCPU;

sem_t mutex_primerNivel;
sem_t mutex_segundoNivel;
sem_t sem_inicializarProceso;
sem_t sem_suspenderProceso;
sem_t sem_finalizarProceso;
sem_t mutex_tablaMarcos;
sem_t mutex_SWAP;
sem_t swapListo;
sem_t sem_handshakeCPU;
sem_t sem_handshakeKerner;

t_list* tablasDePrimerNivel;
t_list* tablasDeSegundoNivel;
t_dictionary* tablaMarcos;
t_dictionary* punterosPorProceso;

typedef struct{
    int puntero;
} punteroID;

t_log* logger;
datosConfig* config;
void* espacioUsuario;

uint32_t cantMarcos;

int fd_memoria_cpu;
int fd_kernel_cpu;
int contador;
//int puntero;

uint32_t ceilCasero(float numero);
uint32_t floorCasero(float numero);
uint32_t minimoEntre(uint32_t a, uint32_t b);
void config_get_string_values();
void init_semaforos();
void init_listas();
void init_espacioUsuario();
void init_tablaMarcos();
uint32_t tamanioArchivo(uint32_t tamProceso);
uint32_t tamanioActualDelArchivo(FILE* archivoAbierto);
void esperarSwap();
char* generarNombreDeArchivo(uint32_t pid);
char* generarPath(uint32_t pid);
void crearArchivo(uint32_t pid, uint32_t tamProceso);
void asignarMarcos(uint32_t pidProceso);
uint32_t generarEstructuras(uint32_t pid, uint32_t tamProceso);
void inicializarProceso(uint32_t pid, uint32_t tamProceso);
void copiarEnDisco(uint32_t pid, uint32_t marco, uint32_t nroPagina);
void sacarPaginaDeMemoria(datosPagNumerada* paginaAEliminar);
void liberarMarcos(uint32_t pidProceso);
void desSuspenderProceso(uint32_t pid);
void suspenderProceso(uint32_t pid, uint32_t tabla_paginas);
void eliminarProcesoDelDisco(uint32_t pid);
void finalizarProceso(uint32_t pid, uint32_t tabla_paginas);
void devolverTablaSegundoNivel(uint32_t nroTabla, uint32_t entrada);
void sacarPaginaDeDisco(uint32_t pid, uint32_t marco, uint32_t nroPagina);
void cargarPaginaEnMemoria(datosPagNumerada* paginaACargar, uint32_t marcoDestino);
void* algoritmo_clock(t_list* listaDatosPagina,uint32_t pid);
void* algoritmo_clock_mejorado(t_list* listaDatosPagina,uint32_t pid);
uint32_t aplicarAlgoritmoReemplazo(t_list* paginasEnMemoria, datosPagNumerada* paginaACargar,uint32_t pid);
uint32_t buscarMarcoLibre(uint32_t pidBuscado);
uint32_t reemplazarPagina(uint32_t pid, datosPagNumerada* paginaACargar);
void agregarPagina(uint32_t pid, datosPagNumerada* paginaACargar);
uint32_t obtenerNumeroDePagina(uint32_t entradaPrimerNivel, uint32_t entradaSegundoNivel);
void devolverMarco(uint32_t pid, uint32_t nroTablaSegundoNivel, uint32_t entradaTablaSegundoNivel);
void leer(uint32_t df);
void escribir(uint32_t df, uint32_t valor);
void* escucharCPU();
void* escucharKernel();
void levantarHilos();
void inicializarMemoria();

int nro_vuelta;

#endif
