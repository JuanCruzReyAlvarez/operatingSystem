#include "auxiliares.h"
/*
uint32_t getProcesosHabilitados() {
	return (atoi(config -> grado_multiprogramacion)) - procesosEnMemoria;
}
*/
void* init_semaforos(){

    int gradoMultiprogramacion = atoi(config -> grado_multiprogramacion);

    sem_init(&sem_ready, 0, 1);
    sem_init(&sem_exec, 0, 1);
    sem_init(&sem_blocked, 0, 1);
    sem_init(&sem_new, 0, 1);
    sem_init(&sem_exit, 0, 1);
    sem_init(&sem_suspended_ready, 0, 1);
    sem_init(&sem_suspended_blocked, 0, 1);
    sem_init(&sem_procesosEnMemoria, 0, 1);
    sem_init(&mutex_io, 0, 1);
    sem_init(&quieroIrAExec, 0, 0);
    sem_init(&procesosBlockedReady, 0, 0);
    sem_init(&procesosSuspBlockedSuspReady, 0, 0);
    sem_init(&listaBloqueadosAlDia, 0, 1);
    sem_init(&procesosEnIO, 0, 0);
    sem_init(&queHayaLugarEnMemoria, 0, gradoMultiprogramacion);
    sem_init(&procesosEnReady, 0, 0);
    sem_init(&llegoPCBExec, 0, 0);
    sem_init(&mutex_cpu_vacia, 0, 1);
    sem_init(&mutex_recv_dispatch, 0, 1);
    sem_init(&quieroIrAMemoria, 0, 0);
    sem_init(&mutex_mandarAReady, 0, 1);
    sem_init(&meQuieroIr, 0, 0);
    sem_init(&procesosSuspReadyReady, 0, 0);
    sem_init(&procesosNewReady, 0, 0);
    sem_init(&unoEnIOALaVez, 0, 1);
    sem_init(&mutex_atenderBloqueo, 0, 1);
    

    return NULL; 
}

//void* crear_listas(t_list* estadoReady,t_list* estadoExec, t_list* estadoBlocked, t_list* estadoNew, t_list* estadoExit, t_list* estadoSuspendedReady,t_list*  estadoSuspendedBlocked){
void* crear_listas(){
    estadoReady = list_create();
    estadoBlocked = list_create();
    estadoNew = list_create();
    estadoExit = list_create();
    estadoSuspendedReady = list_create();
    estadoSuspendedBlocked = list_create();
    

    return NULL;
}

void config_get_string_values(t_config* nuevo_config){

    datosConfig* configNuevo = malloc(sizeof(datosConfig));

    nuevo_config = config_create("kernel.config"); // "cfg/kernel.config"

    char* ip_memoria = config_get_string_value(nuevo_config,"IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(nuevo_config,"PUERTO_MEMORIA");
    char* ip_cpu = config_get_string_value(nuevo_config,"IP_CPU");
    char* puerto_cpu_dispatch = config_get_string_value(nuevo_config,"PUERTO_CPU_DISPATCH");
    char* puerto_cpu_interrupt= config_get_string_value(nuevo_config,"PUERTO_CPU_INTERRUPT");
    char* puerto_escucha = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA");
    char* algoritmo_planificacion = config_get_string_value(nuevo_config,"ALGORITMO_PLANIFICACION");
    char* estimacion_inicial = config_get_string_value(nuevo_config,"ESTIMACION_INICIAL");
    char* alfa = config_get_string_value(nuevo_config,"ALFA");
    char* grado_multiprogramacion = config_get_string_value(nuevo_config,"GRADO_MULTIPROGRAMACION");
    char* tiempo_maximo_bloqueado = config_get_string_value(nuevo_config,"TIEMPO_MAXIMO_BLOQUEADO");
    
    configNuevo -> ip_memoria = ip_memoria;
    configNuevo -> puerto_memoria = puerto_memoria;
    configNuevo -> ip_cpu = ip_cpu;
    configNuevo -> puerto_cpu_dispatch = puerto_cpu_dispatch;
    configNuevo -> puerto_cpu_interrupt = puerto_cpu_interrupt;
    configNuevo -> puerto_escucha = puerto_escucha;
    configNuevo -> algoritmo_planificacion = algoritmo_planificacion ;
    configNuevo -> estimacion_inicial = estimacion_inicial;
    configNuevo -> alfa = alfa;
    configNuevo -> grado_multiprogramacion = grado_multiprogramacion;
    configNuevo -> tiempo_maximo_bloqueado = tiempo_maximo_bloqueado;

    config = configNuevo;    
}

void* agregar_instruccion_lista_de_instrucciones(op_code cop,uint32_t param1,uint32_t param2,t_list* list,int cant_parametros){

    instruccion *nuevaInstrucciones_pcb = malloc(sizeof(instruccion));   
    nuevaInstrucciones_pcb -> id_INSTRUCCION = generarString(cop);
    switch(cant_parametros){
        case 2:
            nuevaInstrucciones_pcb -> param[1] = param2;
        case 1:
            nuevaInstrucciones_pcb -> param[0] = param1;
            break;
        case 0:
            break;
        default:
            log_error(logger,"Hubo un problema a la hora de agregar instrucciones a la lista en el kernel \n");
            break;
    }
    list_add(list,nuevaInstrucciones_pcb);

    //log_info(logger, "Agregue un %s a la lista de instrucciones.", generarString(cop));
    //free(nuevaInstrucciones_pcb);

    return NULL;

}



void* calcularSRT(){ 
    
    uint32_t i;
    sem_wait(&sem_ready);
    pcb* pcb_actual = list_get(estadoReady,0);
    for(i = 1; i < list_size(estadoReady) ;i++){ 
        pcb* pcb_a_comparar = list_get(estadoReady,i);
        
        if((rafaga_restante(pcb_a_comparar) < rafaga_restante(pcb_actual)) || (rafaga_restante(pcb_a_comparar) == rafaga_restante(pcb_actual) && pcb_a_comparar->id == idRecibidoDispatch)) // IMPLEMENTAR ESA FUNCIÓN (estimación) |  menor, entonces si empatan gana FIFO
        pcb_actual = pcb_a_comparar;
    }

    uint32_t indice = id_a_indice(pcb_actual -> id,estadoReady);
    pcb_actual = list_remove(estadoReady,indice);
	sem_post(&sem_ready);

    log_info(logger, "Calcule SRT y gano la PCB %d.", pcb_actual->id);

    return pcb_actual;
}

void decidirProximoEstado(pcb* pcbRecibida) {
    sem_wait(&mutex_io);
	if (pcbRecibida -> estado_proceso == BLOCKED) {
		sem_post(&procesosBlockedReady);
        log_info(logger, "Mande la PCB %d que estaba en I/O a READY.", pcbRecibida->id);
    } else { 
		sem_post(&procesosSuspBlockedSuspReady);
        log_info(logger, "Mande la PCB %d que estaba en I/O a SUSPENDED READY.", pcbRecibida->id);
    }
    sem_post(&unoEnIOALaVez);
    return;
}

void ejecutarIO() {
    pcbIO* pcbAEjecutar;
    while(1) {
        sem_wait(&listaBloqueadosAlDia);
        sem_wait(&procesosEnIO);
        sem_wait(&unoEnIOALaVez);
        sem_wait(&sem_blocked);
        
        pcbAEjecutar = list_get(estadoBlocked,0);    
        sem_post(&sem_blocked);
        uint32_t tiempoDeEspera = (pcbAEjecutar -> tiempoDeBloqueo) * 1000;
        log_info(logger, "Bloqueo la PCB %d durante (%d milisegundos).", pcbAEjecutar->pcbBloqueada->id, pcbAEjecutar->tiempoDeBloqueo);
        
        usleep(tiempoDeEspera);
        
        decidirProximoEstado(pcbAEjecutar->pcbBloqueada);
        //free(pcbAEjecutar);
    }
	return;
}

void chequearSiTengoQueSuspenderlo(pcbIO* pcbIOBloqueada) { // la de muchos hilos

	uint32_t programCounter = pcbIOBloqueada -> pcbBloqueada -> program_counter;
    uint32_t idViejo = pcbIOBloqueada -> pcbBloqueada -> id; 

    
	usleep(atoi(config->tiempo_maximo_bloqueado) * 1000);
	sem_wait(&mutex_io);
	if (pcbIOBloqueada -> pcbBloqueada -> program_counter == programCounter && pcbIOBloqueada -> pcbBloqueada -> estado_proceso == BLOCKED) { 
		pcbIOBloqueada -> pcbBloqueada -> estado_proceso = SUSPENDED_BLOCKED;
		sem_post(&mutex_io);
		sacarPCBDeMemoria(pcbIOBloqueada -> pcbBloqueada); 

        log_info(logger, "Mande la PCB %d a SUSPENDED BLOCKED.", pcbIOBloqueada->pcbBloqueada->id);
	}
    else {
        log_info(logger, "La PCB %d NO supero el tiempo maximo de bloqueo (no la suspendi).", idViejo);
        sem_post(&mutex_io);
    }
	return;
} 

void* agregarAMemoria(pcb* pcbAEnviarAMemoria){

    //log_info(logger,"Mando a memoria el PID es %d y tabla_paginas = %d para \n", pcbAEnviarAMemoria->id, pcbAEnviarAMemoria->tabla_paginas);

    if (pcbAEnviarAMemoria -> tabla_paginas == -1) {
        char* mensajeCreacion = "INICIALIZAR";
        uint32_t numeroDePID = pcbAEnviarAMemoria -> id;
        uint32_t tamProceso = pcbAEnviarAMemoria -> tamanio_bytes;

        if (!send__char_y_dos_int(fd_memoria, mensajeCreacion, numeroDePID, tamProceso))
            log_error(logger, "No se pudo pedir a memoria que cree las estructuras del proceso %d", numeroDePID);
        else {
            uint32_t tablaDePagina;
            uint32_t pidRecibido;

            if(!recv_dos_uint(fd_memoria, &pidRecibido, &tablaDePagina))
                log_error(logger,"No pudo recibir la respuesta al mensaje de creacion, algo anda mal!");
            else {
                if (pidRecibido != numeroDePID)
                    log_error(logger, "Se recibió el PID %d pero se tenía que recibir el %d.", pidRecibido, numeroDePID);
                else {
                    log_info(logger, "Se recibió la tabla de páginas N°%d para el proceso %d.", tablaDePagina, pidRecibido);
                    pcbAEnviarAMemoria -> tabla_paginas = tablaDePagina;
                }
            }
        }
    }
    else {
        char* mensajeCreacion = "NO_SUSPENDED";
        uint32_t numeroDePID = pcbAEnviarAMemoria -> id;
        if (!send__char_y_dos_int(fd_memoria, mensajeCreacion, numeroDePID, 0))
            log_error(logger, "No se pudo pedir a memoria que des-suspenda el proceso %d.", numeroDePID);
        else
            log_info(logger, "Se pidió a memoria que des-suspenda el proceso %d.", numeroDePID);
    }
    return NULL;
}

void* sacarPCBDeMemoria(pcb* pcbASacar) {

    uint32_t numeroDePID = pcbASacar -> id;
    uint32_t tabla = pcbASacar ->tabla_paginas;
    char* mensajeFinalizacion;

    if (pcbASacar -> estado_proceso == FIN ) {
        log_info(logger, "Le mande un mensaje a memoria para que saque a la PCB %d en estado FIN.", pcbASacar -> id );
        mensajeFinalizacion = "FINALIZAR";
    } else {
        log_info(logger, "Le mande un mensaje a memoria para que saque a la PCB %d por suspensión (estado %s).", pcbASacar -> id, generar_string_de_estado(pcbASacar -> estado_proceso) );
        mensajeFinalizacion = "SUSPENDER";
    }


    if(!send__char_y_dos_int(fd_memoria,mensajeFinalizacion,numeroDePID,tabla))
            log_error(logger,"No pude pedirle a memoria que saque a la PCB %d.", pcbASacar -> id);
        else
            log_warning(logger,"Le mande un mensaje a memoria para que saque a la PCB %d", pcbASacar -> id);

    sem_post(&queHayaLugarEnMemoria);
    return NULL;
}

void enviarMensajeDeFinalizacion(pcb* pcbFinalizadaCpu) {
    char* mensajeDeFin = string_new();
    string_append(&mensajeDeFin, "TERMINE_OK - PID ");
    string_append(&mensajeDeFin, string_itoa(pcbFinalizadaCpu -> id));

	send_mensaje_consola(pcbFinalizadaCpu->id,mensajeDeFin);
    return;
}

void interrumpirCPU() {
    
    esperandoDesalojo = true;
    char* mensajeInterrupcion = "ME_INTERRUMPI";
    
    if(!send_mensaje_interrupcion(fd_cpu_interrupt,mensajeInterrupcion))
            log_error(logger, "No se pudo mandar el mensaje %s hubo un fallo",mensajeInterrupcion);
        else 
            log_info(logger, "Se mando el mensaje %s",mensajeInterrupcion);
  
    log_info(logger, "Interrumpi la CPU.");
    
    return;
}

void mandarAReady(pcb* pcbRecibida) {

    sem_wait(&sem_ready);
    list_add(estadoReady, pcbRecibida); 
    sem_post(&sem_ready);

    sem_post(&procesosEnReady);

    if (strcmp(config -> algoritmo_planificacion, "SRT") == 0 && cpuOcupada) {
        interrumpirCPU();
        sem_wait(&llegoPCBExec);
        if (intRecibidoDispatch == -2) {
            idRecibidoDispatch = pcbRecibidaDispatch->id;
            pcbRecibidaDispatch-> estado_proceso = READY;
            sem_wait(&sem_ready);
            list_add(estadoReady, pcbRecibidaDispatch);
            sem_post(&sem_ready);
            sem_post(&procesosEnReady);
            sem_post(&mutex_cpu_vacia);

            log_info(logger, "Mande a READY a la PCB %d.", pcbRecibida->id);

        }
        else {
            log_warning(logger,"Quise desalojar pero la PCB volvio antes por EXIT o BLOCKED o no había nadie en CPU");
        }
    }
    return;
}


t_list* copiarListaInstrucciones( t_list* origen) {
   
    t_list* listaInstrucciones = list_create();
    
    int j = list_size(origen);
    

    for(int i=0; i<j; i++) {
        
        instruccion* instDestino = list_remove(origen, 0);
        
        list_add(listaInstrucciones, (void*)instDestino);
    }
    
    list_destroy_and_destroy_elements(origen, free);
    
    return listaInstrucciones;
} 


void copiarPCB(pcb* destino, pcb* pcbRecibidaDispatch) {
    
    destino -> id = pcbRecibidaDispatch -> id;
    destino -> tamanio_bytes = pcbRecibidaDispatch -> tamanio_bytes;

    t_list* listaInstruccionesDes = malloc(sizeof(t_list));
    listaInstruccionesDes = copiarListaInstrucciones(pcbRecibidaDispatch->listaInstrucciones);
    destino -> listaInstrucciones = listaInstruccionesDes;
    destino -> estado_proceso = pcbRecibidaDispatch -> estado_proceso;
    destino -> program_counter = pcbRecibidaDispatch -> program_counter;
    destino -> tabla_paginas = pcbRecibidaDispatch -> tabla_paginas;
    destino -> estimacion_rafaga = pcbRecibidaDispatch -> estimacion_rafaga;
    return;
}

pcbIO* inicializarPCBIO() {
    

    pcbIO* pcbAMandar = malloc(sizeof(pcbIO));
    pcb* pcbABloquear = malloc(sizeof(pcb));
    
    copiarPCB(pcbABloquear, pcbRecibidaDispatch);

    pcbAMandar -> pcbBloqueada = pcbABloquear;
    pcbAMandar -> tiempoDeBloqueo = intRecibidoDispatch;

    free(pcbRecibidaDispatch);
    return pcbAMandar;
}

void atenderBloqueo() {
    sem_wait(&mutex_atenderBloqueo);

    pcbIO* pcbABloquear = inicializarPCBIO();

    sem_wait(&sem_blocked);
    list_add(estadoBlocked, pcbABloquear);
    sem_post(&sem_blocked);

    log_info(logger, "Atendiendo el bloqueo de la PCB %d por %d milisegundos.", pcbABloquear->pcbBloqueada->id, pcbABloquear->tiempoDeBloqueo);

    
	pthread_t hilo_chequearSiTengoQueSuspenderlo;
	pthread_create(&hilo_chequearSiTengoQueSuspenderlo, NULL, (void*)chequearSiTengoQueSuspenderlo, pcbABloquear);
	pthread_detach(hilo_chequearSiTengoQueSuspenderlo);
    
	sem_post(&procesosEnIO);
    sem_post(&mutex_atenderBloqueo);
	return;
}

void conexionesPorDispatch() {   
    while(1) {

        sem_wait(&quieroIrAExec);
        sem_wait(&sem_exec);

        estadoExec -> estado_proceso = EXEC;

    
        send_PCB(fd_cpu_dispatch, estadoExec);
        cpuOcupada = true;

        log_info(logger, "Mande la PCB %d a EXEC.", estadoExec -> id);

        free(estadoExec);
        
        sem_post(&sem_exec);

        sem_wait(&mutex_recv_dispatch);

        pcbRecibidaDispatch = malloc(sizeof(pcb));
        intRecibidoDispatch = recv_PCB_mas_int(fd_cpu_dispatch, pcbRecibidaDispatch); //si se cae la conexion, aca va a mostrar lo que tenga la ultima PCB que recibio
        if (intRecibidoDispatch == -1) {
            log_error(logger, "Error al recibir una PCB por dispatch");
            exit(-1);
        }
        cpuOcupada = false;
        log_warning(logger, "DISPATCH: Recibi la PCB %d con el estado %s.", pcbRecibidaDispatch->id, generar_string_de_estado(pcbRecibidaDispatch->estado_proceso));
        
        switch (pcbRecibidaDispatch -> estado_proceso) {
            case EXEC:
                if (esperandoDesalojo) {
                    sem_post(&llegoPCBExec);
                }
                else {
                    log_error(logger, "RECIBI UNA PCB EN ESTADO EXEC CUANDO NADIE LA DESALOJO");
                    sem_post(&mutex_cpu_vacia);
                }
                break;
            case BLOCKED:
                if (esperandoDesalojo) {
                    sem_post(&llegoPCBExec);
                }
                else {
                    sem_post(&mutex_cpu_vacia);
                }
                atenderBloqueo();
                
                break;
            case FIN:
                if (esperandoDesalojo) {
                    sem_post(&llegoPCBExec);
                }
                else {
                    //sem_post(&mutex_cpu_vacia);
                }
                sem_wait(&sem_exit);
                list_add(estadoExit,pcbRecibidaDispatch);
                sem_post(&sem_exit);
                sem_post(&meQuieroIr);
                sem_post(&mutex_cpu_vacia);
                break;
            default:
                log_error(logger, "Recibi una PCB con un estado que no es EXEC, EXIT o BLOCKED");
        }
        esperandoDesalojo = false;
   //     sem_post(&turnoCPU);
        sem_post(&mutex_recv_dispatch);
    }
}

void peleaEntreNewSuspReady() {
    if (list_is_empty(estadoSuspendedReady)) {
        log_info(logger, "Voy a mandar una PCB nueva a memoria.");
        sem_post(&procesosNewReady);
    }
    else {
        log_info(logger, "Voy a mandar a memoria una PCB que paso de suspReady a ready");
        sem_post(&procesosSuspReadyReady);
    }
}

void mandarAMemoria(){
    while(1){
        //log_warning(logger,"Esperando mandarAMemoria()");
        sem_wait(&quieroIrAMemoria);
        //log_warning(logger, "Pase &quieroIrAMemoria. Esperando &queHayaLugarEnMemoria.");
        sem_wait(&queHayaLugarEnMemoria);

        log_info(logger, "Se activo mandarAMemoria()");


        peleaEntreNewSuspReady();
    }
}

void* calcular_FIFO() {
    // elijo el primer proceso de la cola READY
    sem_wait(&sem_ready);
    pcb* pcb_actual = list_remove(estadoReady,0);
    sem_post(&sem_ready);
    
    log_info(logger, "Calcule FIFO y gano la PCB %d", pcb_actual->id);

    return pcb_actual;
}

