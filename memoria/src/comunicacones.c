#include "comunicaciones.h"

typedef struct {
    t_log* log;   // que es esto? Y por que no esta inicializado
    int fd;
    char* server_name;
} t_procesar_conexion_args;


bool generar_conexion_memoria_kerner(t_log* logger, int* fd_kerner) {

   
    t_config* nuevo_config;
	nuevo_config = config_create("memoria.config");

    char* puerto_escucha;

    puerto_escucha = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA");


    *fd_kerner = crear_conexion(logger,"MEMORIA_A_KERNEL",NULL, puerto_escucha); // FALLA PORQUE NO ESTÁ ABIERTA LA CONEXIÓN DEL QUERIDO KERNER

    config_destroy(nuevo_config);

    //printf("\n\n\n%d   -   %d\n\n\n", *fd_dispatch,*fd_memoria);

    return *fd_kerner != 0;
}

void handshake_con_cpu(){
    t_config* nuevo;
    nuevo = config_create("memoria.config");
    uint32_t tamanio_pagina = (uint32_t) (atoi(config_get_string_value(nuevo,"TAM_PAGINA")));
    uint32_t entradas_por_tab = (uint32_t) (atoi(config_get_string_value(nuevo,"ENTRADAS_POR_TABLA")));
    
    if(!send_dos_uint(fd_memoria_cpu,tamanio_pagina,entradas_por_tab))
       log_error(logger,"OCURRIO UN PROBLEMA EN EL HANDSHAKE!!!!!!!!!");
    else {
        log_warning(logger,"Mande a cpu el tamaño de pagina y la cantidad de entradas por tabla");
	sem_post(&sem_handshakeCPU);
    }
    config_destroy(nuevo);
}

/*
void mensaje_recibido_por_kernel(){
    char* mensajeRec;
    uint32_t mensajeRecibido;

    char* mensajeAEnviar;
    uint32_t tablaDePaginas;

    recv_mensaje_memoria(fd_kernel_cpu,&mensajeRec,&mensajeRecibido);

        log_info(logger,"Se recibio %s \n",mensajeRec);

        if(strcmp(mensajeRec,"CREAR_ESTRUCTURAS") == 0) {

            mensajeAEnviar = "LA TABLA_DE_PAG ES: ";
            tablaDePaginas = (uint32_t) (rand() % 100);
        log_info(logger,"EL FD_MEMORIA ES IGUAL A: %d \n",fd_kernel_cpu);

        if(!send_mensaje_memoria(fd_kernel_cpu,mensajeAEnviar,tablaDePaginas))
            log_info(logger,"No se manda el mensaje porque no esta hecho el cliente aca \n");
        else
            log_info(logger,"Anduvo joya y mande las cosas!");

        }

        else if(strcmp(mensajeRec,"DESTRUIR_ESTRUCTURAS") == 0) {
       
            log_info(logger,"VOY A BORRAR LAS ESTRUCTURAS EN EL PROXIMO CHECKPOINT");

        }

}

*/

static void procesar_conexion_kernel(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;

    printf("Nombre del server: %s \n", server_name);
    printf("Socket número: %d \n", cliente_socket);
    //printf("Hasta acá\n"); // prueba de errores

    //while(1){ 

        escucharKernel();

    
	if(cliente_socket == -1) {
		log_warning(logger, "Se cerró la conexión.\n");
    }

    //log_warning(logger, "Dejé de escuchar en %s.", server_name);
    return;

    
}

static void procesar_conexion_cpu(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;

    printf("Nombre del server: %s \n", server_name);
    printf("Socket número: %d \n", cliente_socket);
    //printf("Hasta acá\n"); // prueba de errores

    if(cliente_socket == fd_memoria_cpu){
        handshake_con_cpu();
    }

    //while(1){

    escucharCPU();

    
	if(cliente_socket == -1) {
		log_warning(logger, "Se cerró la conexión.\n");
        //break;
        }
    //}

    //log_warning(logger, "Dejé de escuchar en %s.", server_name);
    return;

    
}

void conectarseAKernelMemoria(){
    fd_kerner = 0;

    if (!generar_conexion_memoria_kerner(logger, &fd_kerner)) {       //ver este cliente
        log_destroy(logger);
        //return EXIT_FAILURE;
        log_error(logger,"HUBO UN ERROR AL GENERAR LAS CONEXIONES");
    } else
	sem_post(&sem_handshakeKerner);
}


int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    if(contador == 0){
    fd_memoria_cpu = cliente_socket;
    contador++;
    }
    else
    fd_kernel_cpu = cliente_socket;

    if (cliente_socket != -1 && cliente_socket == fd_memoria_cpu) {
        log_info(logger, "Creando hilo.");
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion_cpu, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    else if (cliente_socket != -1 && cliente_socket == fd_kernel_cpu){
        log_info(logger, "Creando hilo.");
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion_kernel, (void*) args);
        pthread_detach(hilo);
        return 1;
    }

    return 0;
}
