#include "comunicaciones.h"


op_code recibir_cop_PRUEBA(int socket_cliente) { 
    op_code cop;
    if (recv(socket_cliente, &cop, sizeof(op_code), MSG_WAITALL) != 0) {
        
        return cop;
    }
    else {
        return -1;
    }
}

op_code recibir_cop(int socket_cliente) {
    op_code cop;
    if (recv(socket_cliente, &cop, sizeof(op_code), MSG_WAITALL) != sizeof(op_code)) {
        log_error(logger,"Error en el recv().\n");
        return -1;
    }
	return cop;
}

bool generar_conexiones_kerner(t_log* logger, int* fd_dispatch,int* fd_memoria, int* fd_interrupt) {
    char* ip_cpu;
    char* puerto_dispatch;
    char* puerto_interrupt;
    char* ip_memoria;
    char* puerto_memoria;

    ip_cpu = config -> ip_cpu;
    puerto_dispatch = config -> puerto_cpu_dispatch;
    puerto_interrupt = config -> puerto_cpu_interrupt;
    
    ip_memoria = config -> ip_memoria;
    puerto_memoria = config -> puerto_memoria;

    *fd_dispatch = crear_conexion(logger,"CPU_DISPATCH",ip_cpu, puerto_dispatch); 
    *fd_memoria = crear_conexion(logger,"MEMORIA",ip_memoria, puerto_memoria);
    *fd_interrupt = crear_conexion(logger,"CPU_INTERRUPT",ip_cpu, puerto_interrupt);


    return *fd_dispatch != 0 && *fd_memoria != 0 && *fd_interrupt!=0;
}

uint32_t read_estimation_from_config(){
        t_config* nuevo_config;
        uint32_t estimacion;
        nuevo_config = config_create("kernel.config"); 
        estimacion = (uint32_t)atoi(config_get_string_value(nuevo_config,"ESTIMACION_INICIAL"));
        config_destroy(nuevo_config);
        return estimacion;
    }

pcb* estado_NEW(t_list* list_instrucciones,uint32_t tamanioProceso,int cliente_socket){  // fue hecho en su momento para hacer pruebas 

    pcb *pcb_inicial = malloc(sizeof(pcb));

    pcb_inicial -> id = cliente_socket;

    pcb_inicial -> tamanio_bytes = tamanioProceso;

    pcb_inicial ->  listaInstrucciones = list_instrucciones;

    pcb_inicial -> estado_proceso = NEW;

    pcb_inicial -> program_counter = 0;

    uint32_t estimacion = read_estimation_from_config();

    pcb_inicial -> estimacion_rafaga = estimacion;

    return pcb_inicial;

}

void* nullANew(t_list* list_instrucciones,uint32_t tamanioProceso,int cliente_socket){

    pcb *pcb_inicial = malloc(sizeof(pcb));

    pcb_inicial -> id = cliente_socket;

    pcb_inicial -> tamanio_bytes = tamanioProceso;

    pcb_inicial ->  listaInstrucciones = list_instrucciones;

    pcb_inicial -> estado_proceso = NEW;

    pcb_inicial -> program_counter = 0;

    uint32_t estimacion = read_estimation_from_config();

    pcb_inicial -> estimacion_rafaga = estimacion;

    pcb_inicial -> tabla_paginas = -1;
    
    

    sem_wait(&sem_new);
    list_add(estadoNew,pcb_inicial);
    sem_post(&sem_new);
    sem_post(&quieroIrAMemoria);

    return NULL;

}

static void procesar_conexion(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;

    log_info(logger,"Nombre del server: %s \n", server_name);
    log_info(logger,"Socket número: %d \n", cliente_socket);
    //printf("Hasta acá\n"); // prueba de errores

    op_code cop; // hay que ver si lo inicializamos con algo (pipe: yo creería que no)
	if(cliente_socket == -1)
		log_info(logger,"Se cerró la conexión.\n");
    
    uint32_t tamnProceso;
    if(recv(cliente_socket, &tamnProceso, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t))
        log_info(logger,"No se pudo leer el tamanio del proceso\n");

    log_info(logger,"EL TAMANIO DEL PROCESO ES %d \n",tamnProceso);

    cop = recibir_cop(cliente_socket);
    
    t_list* listaInstrucciones = list_create();

    while (cop != EXIT) {
        
        switch (cop) {
            

            case WRITE:
            case COPY:
            {  
                
                uint32_t param1;  
                uint32_t param2;

                if (!recv_dos_parametros(cliente_socket,&param1,&param2)) {
                    log_error(logger, "Fallo recibiendo DOS PARAMETROS.");
                    break;
                }

                agregar_instruccion_lista_de_instrucciones(cop,param1,param2,listaInstrucciones,2); 

                break;
            }

            case IO:
            case READ:
            {  
                
                uint32_t param1;


                if (!recv_un_parametro(cliente_socket,&param1)) {
                    log_error(logger, "Fallo recibiendo UN PARAMETRO y sacando la basura.");
                    break;
                }

                
                agregar_instruccion_lista_de_instrucciones(cop,param1,param1,listaInstrucciones,1); 
                

                break;
            }

            case NO_OP:
            {
            if(!recv_cero_parametros(cliente_socket)) {
                log_error(logger, "Fallo recibiendo CERO PARÁMETROS en NO_OP y sacando la basura.");
            }
            
            agregar_instruccion_lista_de_instrucciones(cop,0,0,listaInstrucciones,0);
            
            break;
            }
            
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_error(logger, "Cop: %s", generarString(cop));
                break;
        }

        cop = recibir_cop(cliente_socket);
        //log_warning(logger,"EL SEGUNDO QUE RECIBI ES %s", generarString(cop));
    }

    
	if (!recv_cero_parametros(cliente_socket))
        log_error(logger, "Fallo sacando la basura.");
	else{
        uint32_t param1 = 0;
       
        agregar_instruccion_lista_de_instrucciones(cop,param1,param1,listaInstrucciones,0); 
        
    }

    nullANew(listaInstrucciones,tamnProceso,cliente_socket);
    

    //log_warning(logger, "Dejé de escuchar en %s.", server_name);
    if(list_size(estadoBlocked) > 0) {
        //pcbIO* pcbIOBlocked = list_get(estadoBlocked, 0);
        //log_error(logger, "El primer elemento de la lista es la PCB %d", pcbIOBlocked->pcbBloqueada->id);
    } else {
        //log_error(logger, "El tamanio de la lista es 0");
    }
    return;

    
}

void conectarse_a_memoria_cpu() {
    fd_cpu_dispatch=0;
    //fd_memoria=0;
    fd_cpu_interrupt=0;
    if (!generar_conexiones_kerner(logger, &fd_cpu_dispatch,&fd_memoria,&fd_cpu_interrupt)) {                                      
        log_destroy(logger);
        
        log_error(logger,"HUBO UN ERROR AL GENERAR LAS CONEXIONES");
    }
    printf("FD_MEMORIA EN CONECTARSE VALE: %d \nFD_CPU_DISPATCH EN CONECTARSE VALE: %d\n\n\n\n",fd_memoria, fd_cpu_dispatch);
}

void handshake_de_alfa(){
    float alfa_a_enviar = atof(config -> alfa);
    printf("FD: %d\n\nALFA: %f\n\n", fd_cpu_dispatch, alfa_a_enviar);
    if(!send(fd_cpu_dispatch,&alfa_a_enviar,sizeof(float),0))
        log_error(logger,"OCURRIO UN PROBLEMA EN EL HANDSHAKE!");
    else
        log_info(logger,"mande el alfa en el handshake");
}


int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    fd_kernel_a_consola = cliente_socket;

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

