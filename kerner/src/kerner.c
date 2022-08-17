#include "kerner.h"

// ctrl+k contraseña chmod 777 exec

void levantarHilos() {
    // ejecutarIO();
    pthread_t hilo_ejecutarIO;
    pthread_create(&hilo_ejecutarIO, NULL, (void*)ejecutarIO, NULL);
    pthread_detach(hilo_ejecutarIO);

    // conexionesPorDispatch();
    pthread_t hilo_conexionesPorDispatch;
    pthread_create(&hilo_conexionesPorDispatch, NULL, (void*)conexionesPorDispatch, NULL);
    pthread_detach(hilo_conexionesPorDispatch);

    // mandarAMemoria();
    pthread_t hilo_mandarAMemoria;
    pthread_create(&hilo_mandarAMemoria, NULL, (void*)mandarAMemoria, NULL);
    pthread_detach(hilo_mandarAMemoria);

    // blocked_Ready()
    pthread_t hilo_blocked_Ready;
    pthread_create(&hilo_blocked_Ready, NULL, (void*)blocked_Ready, NULL);
    pthread_detach(hilo_blocked_Ready);

    // planificarCortoPlazo();
    pthread_t hilo_planificarCortoPlazo;
    pthread_create(&hilo_planificarCortoPlazo, NULL, (void*)planificarCortoPlazo, NULL);
    pthread_detach(hilo_planificarCortoPlazo);

    // suspended_blocked_Suspended_ready()
    pthread_t hilo_suspended_blocked_Suspended_ready;
    pthread_create(&hilo_suspended_blocked_Suspended_ready, NULL, (void*)suspended_blocked_Suspended_ready, NULL);
    pthread_detach(hilo_suspended_blocked_Suspended_ready);

    // execAExit()
    pthread_t hilo_execAExit;
    pthread_create(&hilo_execAExit, NULL, (void*)execAExit, NULL);
    pthread_detach(hilo_execAExit);

    // suspendedReady_Ready();
    pthread_t hilo_suspendedReady_Ready;
    pthread_create(&hilo_suspendedReady_Ready, NULL, (void*)suspendedReady_Ready, NULL);
    pthread_detach(hilo_suspendedReady_Ready);

    // newAReady();
    pthread_t hilo_newAReady;
    pthread_create(&hilo_newAReady, NULL, (void*)newAReady, NULL);
    pthread_detach(hilo_newAReady);

    log_info(logger, "Hilos levantados.");
}


// Que tengan muy buen dia , y suerte con el kernel cracks
// 

int main() {

    printf("EL PID DEL KERNER ES %d\n", getpid());

    cpuOcupada = false;
    esperandoDesalojo = false; 
    
    logger = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_INFO);

	nuevo_config = config_create("kernel.config"); 
    config_get_string_values(nuevo_config);

    crear_listas();
    int fd_kernel = iniciar_servidor(logger, "KERNEL", NULL, config -> puerto_escucha);
    printf("LA IP DE MEMORIA ES: %s\nLA IP DE CPU ES %s\n", config->ip_memoria, config->ip_cpu);
    conectarse_a_memoria_cpu();
    handshake_de_alfa();
    log_info(logger,"Conectado a memoria y CPU");
    init_semaforos();
    levantarHilos();
    
    while (server_escuchar(logger, "KERNEL", fd_kernel));

    log_destroy(logger);
    config_destroy(nuevo_config); 
    free(config);
    free(pcbRecibidaDispatch);
}