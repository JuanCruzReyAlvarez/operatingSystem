#include "memoria.h"
// ctrl+m contraseña chmod 777 exec
int main(){

    //printf("EL PID DE LA MEMORIA ES %d\n", getpid());

    logger = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    //log_info(logger, "Soy la memoria! %s", mi_funcion_compartida());

    tablasCPU = malloc(sizeof(datosProcesoCPU));
    
    contador =0;
    nro_vuelta = 0;


    punterosPorProceso = malloc(sizeof(t_dictionary));
    punterosPorProceso = dictionary_create();
    inicializarMemoria();
    cantMarcos = minimoEntre(config -> entradas_por_tabla * config->entradas_por_tabla, config->marcos_por_proceso);


    log_info(logger, "El tamanio de la pagina es: %d", config -> tam_pagina);
    log_info(logger, "La cantidad de marcos por proceso es: %d", config -> marcos_por_proceso);
    log_info(logger, "La cantidad de entradas por tabla es: %d", config -> entradas_por_tabla);
    log_info(logger, "El algoritmo de reemplazo es: %s", config -> algoritmo_reemplazo);
    int fd_kernel = iniciar_servidor(logger, "MEMORIA", NULL, config -> puerto_escucha);
    while (server_escuchar(logger, "MEMORIA", fd_kernel));
    log_destroy(logger);
    dictionary_clean_and_destroy_elements(punterosPorProceso,free);
}