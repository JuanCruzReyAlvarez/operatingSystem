#include "comunicaciones.h"
#include "ciclo_cpu.h"

typedef struct {
    t_log* log;   // que es esto? Y por que no esta inicializado
    int fd;
    char* server_name;
} t_procesar_conexion_args;



bool generar_conexiones(t_log* logger, int* fd_cpu_a_kernel) {
    char* puerto_kernel;

    t_config* nuevo_config;
	nuevo_config = config_create("cfg/cpu.config"); // "cfg/consola.config"

    puerto_kernel = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA_DISPATCH");

    *fd_cpu_a_kernel = crear_conexion(logger,"CPU_A_KERNEL",NULL, puerto_kernel); // FALLA PORQUE NO ESTÁ ABIERTA LA CONEXIÓN DEL QUERIDO KERNER

    config_destroy(nuevo_config);

    return *fd_cpu_a_kernel != 0;
}


bool generar_conexion_a_memoria(t_log* logger, int* fd_cpu_a_memoria) {
    char* puerto_memoria;
    char* ip_memoria;

    t_config* nuevo_config;
	nuevo_config = config_create("cfg/cpu.config"); // "cfg/consola.config"

    puerto_memoria = config_get_string_value(nuevo_config,"PUERTO_MEMORIA");
    ip_memoria = config_get_string_value(nuevo_config,"IP_MEMORIA");

    *fd_cpu_a_memoria = crear_conexion(logger,"CPU_A_MEMORIA",ip_memoria, puerto_memoria); // FALLA PORQUE NO ESTÁ ABIERTA LA CONEXIÓN DEL QUERIDO KERNER

    config_destroy(nuevo_config);

    return *fd_cpu_a_memoria != 0;
}

void handshake_recibido(){

    if(!recv_dos_uint(fd_cpu_a_memoria,&tamanio_de_la_pagina,&cantidadEntradastablaDePagina))
       log_error(logger,"OCURRIO UN PROBLEMA AL RECIBIR EN EL HANDSHAKE!!!!!!!!!");
    else
        log_warning(logger,"HANDSHAKE REALIZADO : RECIBI EL TAMANIO DE PAGINA %d Y LAS ENTRADAS POR TABLA %d",tamanio_de_la_pagina,cantidadEntradastablaDePagina);

}

void conectarse_a_memoria() {
    fd_cpu_a_memoria=0;

    if (!generar_conexion_a_memoria(logger, &fd_cpu_a_memoria)) {                                      // acá falla y rompe
        log_destroy(logger);
        //return EXIT_FAILURE;
        log_error(logger,"HUBO UN ERROR AL GENERAR LA CONEXION CON MEMORIA");
    }
}


void conectarse_a_kerner_de_cpu() {
    fd_cpu_a_kernel=0;
    if (!generar_conexiones(logger, &fd_cpu_a_kernel)) {                                      // acá falla y rompe
        log_destroy(logger);
        //return EXIT_FAILURE;
        log_error(logger,"HUBO UN ERROR AL GENERAR LAS CONEXIONES");
    }
}


// 
static void procesar_conexion(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    //char* server_name = args->server_name;

    //printf("Nombre del server: %s \n", server_name);
    //printf("Socket número: %d \n", cliente_socket);
    //printf("Hasta acá\n"); // prueba de errores

    pcb* pcbRecibida = malloc(sizeof(pcb));

    int a;
    if((a = recv(cliente_socket,&alfa_recibido,sizeof(float),MSG_WAITALL)) != sizeof(float)) {
        log_error(logger, "No pude recibir el alfa. Recibí %d bytes.", a);
        return;
    } else
        log_info(logger,"Se recibió el alfa con valor: %f", alfa_recibido);

    if(!recv_PCB(cliente_socket,pcbRecibida))
        log_error(logger,"No pudo recibir el pcb, algo anda mal!");
    else
        log_info(logger,"SE RECIBIO LA PCB POR SOCKET : %d", cliente_socket);

    printf("El socket es %d\n", cliente_socket);
    printf(" ID  PCB ES: %d \n", pcbRecibida -> id);
    printf("TAMANIO EN BYTES = %d\n", pcbRecibida->tamanio_bytes);
    /*
    t_list* lista = pcbRecibida -> listaInstrucciones;
    instruccion* inst0 = list_get(lista,0);
    instruccion* inst1 = list_get(lista,1);
    instruccion* inst2 = list_get(lista,2);
    instruccion* inst3 = list_get(lista,3);
    instruccion* inst4 = list_get(lista,4);
    instruccion* inst6 = list_get(lista,6);
    instruccion* inst5 = list_get(lista,5);
    printf("INST.0 = %s %d %d\n", inst0 -> id_INSTRUCCION, inst0 -> param[0], inst0 -> param[1]);
    printf("INST.1 = %s %d %d\n", inst1 -> id_INSTRUCCION, inst1 -> param[0], inst1 -> param[1]);
    printf("INST.2 = %s %d %d\n", inst2 -> id_INSTRUCCION, inst2 -> param[0], inst2 -> param[1]);
    printf("INST.3 = %s %d %d\n", inst3 -> id_INSTRUCCION, inst3 -> param[0], inst3 -> param[1]);
    printf("INST.4 = %s %d %d\n", inst4 -> id_INSTRUCCION, inst4 -> param[0], inst4 -> param[1]);
    printf("INST.6 = %s %d %d\n", inst6 -> id_INSTRUCCION, inst6 -> param[0], inst6 -> param[1]);
    printf("INST.5 = %s %d %d\n", inst5 -> id_INSTRUCCION, inst5 -> param[0], inst5 -> param[1]);
    printf("ESTADO = %s\n", generar_string_de_estado(pcbRecibida->estado_proceso));
    printf("PC = %d\n", pcbRecibida->program_counter);
    printf("TABLA = %d\n", pcbRecibida->tabla_paginas);
    printf("ESTIMACION = %d\n", pcbRecibida->estimacion_rafaga);
    */

    //usleep(30000000);
    
	
    t_inicial = time(NULL);
    iniciar_CPU(pcbRecibida,logger);
    free(pcbRecibida);//  VER QUE ONDA ESTO, PORQUE POR AHI HAY UN FREE EN ALGUNA DE LAS FUNCIONES DE SERIALIZAR Y NO LA ESTAMOS VIENDO
    //log_warning(logger, "Dejé de escuchar en %s.", server_name);

    return;
}



int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    fd_dispatch = cliente_socket;

    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

typedef struct {
    t_log* log;   // que es esto? Y por que no esta inicializado
    int fd;
    char* server_name;
} t_procesar_conexion_args_interrupt;


static void procesar_conexion_interrupt(void* void_args) {
    t_procesar_conexion_args_interrupt* args = (t_procesar_conexion_args_interrupt*) void_args;
    t_log* logger = args->log;
    //int cliente_socket = args->fd;
    //char* server_name = args->server_name;

    //printf("Nombre del server: %s \n", server_name);
    //printf("Socket número: %d \n", cliente_socket);
    //printf("Hasta acá\n"); // prueba de errores

    log_info(logger,"CONEXION CON EXITO PARA PROCESAR INTERRUPCIONES.");
    //free(pcbRecibida);  VER QUE ONDA ESTO, PORQUE POR AHI HAY UN FREE EN ALGUNA DE LAS FUNCIONES DE SERIALIZAR Y NO LA ESTAMOS VIENDO
    //log_warning(logger, "Dejé de escuchar en %s.", server_name);
    return;
}


int server_escuchar_interrupt(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    fd_interrupt = cliente_socket;

    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args_interrupt* args = malloc(sizeof(t_procesar_conexion_args_interrupt));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion_interrupt, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}
