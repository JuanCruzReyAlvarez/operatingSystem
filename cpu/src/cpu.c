#include "cpu.h"
// ctrl+u contraseña chmod777 exec
int main(){

    //printf("EL PID DE LA CPU ES %d\n", getpid());

    logger = log_create("./cfg/cpu.log", "CPU", true, LOG_LEVEL_INFO);   
    logger_mmu = logger;
    logger_tlb = logger;

    char* puerto_dispatch; 
    char* puerto_interrupt;


    t_config* nuevo_config;
	nuevo_config = config_create("cfg/cpu.config"); // "cfg/kernel.config"

    puerto_dispatch = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA_DISPATCH");
    puerto_interrupt = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA_INTERRUPT");

    printf("LA IP DE MEMORIA ES %s\n", config_get_string_value(nuevo_config, "IP_MEMORIA") );

    // hay que sacar el hardcodeado, ver de donde sale ese config
    int fd_dispatch = iniciar_servidor(logger, "CPU", NULL, puerto_dispatch); // VER ESTO de la ip cambiada
    int fd_interrupt = iniciar_servidor(logger, "CPU", NULL, puerto_interrupt);  // VER ESTO de la ip cambiada

    rowTLB = dictionary_create();
    timeStateTablaDirecciones = list_create();
    listLRU = list_create();
    listSeguridadTLB = list_create();
    //limiteTabla = readEntradasTLBFromConfig();
    config_get_string_values(nuevo_config);
    
    conectarse_a_memoria();
    handshake_recibido();

    while ((server_escuchar(logger, "CPU", fd_dispatch)) && (server_escuchar_interrupt(logger, "CPU", fd_interrupt)));
    
    config_destroy(nuevo_config);
    list_destroy_and_destroy_elements(listLRU,free);
    list_destroy_and_destroy_elements(listSeguridadTLB,free);
    list_destroy_and_destroy_elements(timeStateTablaDirecciones,free);
    dictionary_clean_and_destroy_elements(rowTLB,free);
    /*
    instruccion* instdos = malloc(sizeof(instruccion));
    instdos -> id_INSTRUCCION  = "I/O";
    instdos -> param[0] = 1;

    

    instruccion* insttres = malloc(sizeof(instruccion));
    insttres -> id_INSTRUCCION  = "EXIT";
    insttres -> param[0] = 1;

    t_list* instvoldos = list_create();
    list_add(instvoldos,instdos);
    list_add(instvoldos,insttres);




    instruccion* instruccion_uno = malloc(sizeof(instruccion));
    instruccion_uno -> id_INSTRUCCION  = "NO_OP";
    instruccion_uno -> param[0] = 1;


    instruccion* instruccion_dos = malloc(sizeof(instruccion));
    instruccion_dos -> id_INSTRUCCION  = "NO_OP";
    instruccion_dos -> param[0] = 1;

    instruccion* instruccion_tres = malloc(sizeof(instruccion));
    instruccion_tres -> id_INSTRUCCION  = "NO_OP";
    instruccion_tres-> param[0] = 1;

    instruccion* instruccion_cuatro = malloc(sizeof(instruccion));
    instruccion_cuatro -> id_INSTRUCCION  = "IO";
    instruccion_cuatro -> param[0] = 1;

    instruccion* instruccion_cinco = malloc(sizeof(instruccion));
    instruccion_cinco -> id_INSTRUCCION  = "EXIT";
    instruccion_cinco -> param[0] = 1;

    t_list* inst = list_create();
    list_add(inst,instruccion_uno);
    list_add(inst,instruccion_dos);
    list_add(inst,instruccion_tres);
    list_add(inst,instruccion_cuatro);
    list_add(inst,instruccion_cinco);

    
    int i =0 ;
   while(i<=4){
   instruccion* obj = malloc(sizeof(instruccion));
   obj = list_get(inst,i);
   printf("instruccion= %s \n", obj -> id_INSTRUCCION);
   i++;
   free(obj);
   }
    
    pcb* pcb_prueba = malloc(sizeof(pcb));
    pcb_prueba->id = 1;
    pcb_prueba->tamanio_bytes = 1;
    pcb_prueba->listaInstrucciones = inst;        
    pcb_prueba->estado_proceso = EXEC;
    pcb_prueba->program_counter = 0;
    pcb_prueba->tabla_paginas = 0;
    pcb_prueba->estimacion_rafaga = 5000;

    pcb* pcb_pruebados = malloc(sizeof(pcb));
    pcb_pruebados->id = 1;
    pcb_pruebados->tamanio_bytes = 1;
    pcb_pruebados->listaInstrucciones = instvoldos;        
    pcb_pruebados->estado_proceso = EXEC;
    pcb_pruebados->program_counter = 0;
    pcb_pruebados->tabla_paginas = 0;
    pcb_pruebados->estimacion_rafaga = 5000;

    




    iniciar_CPU(pcb_pruebados, logger); 


    free(instruccion_uno);
    free(instruccion_dos);
    free(instruccion_tres);
    free(instdos);
    free(insttres);
    free(pcb_prueba);
    free(pcb_pruebados);
    



    */
    
    log_destroy(logger);
    config_destroy(nuevo_config);
    free(config);


}