#include "corto_plazo.h"

void* blocked_Ready() {
	while(1){
		sem_wait(&procesosBlockedReady);
		//log_info(logger,"BLOCKED-READY");
		pcbIO* pcbIONew;
		pcb* pcbNew;

		sem_wait(&sem_blocked);
		pcbIONew = list_remove(estadoBlocked,0);
		sem_post(&sem_blocked);
		sem_post(&listaBloqueadosAlDia);

		pcbNew = pcbIONew -> pcbBloqueada;
	
		//log_info(logger,"en blocked_Ready, el id es %d", pcbNew->id);
		pcbNew -> estado_proceso = READY;
		sem_post(&mutex_io);

		sem_wait(&mutex_mandarAReady);
		mandarAReady(pcbNew);
		sem_post(&mutex_mandarAReady);

		log_info(logger, "Mande la PCB %d de BLOCKED a READY", pcbNew->id);
		//log_info(logger, " FIN BLOCKED-READY");
		
	}
	return NULL;
}

void planificarCortoPlazo() {
    while(1) {
        sem_wait(&procesosEnReady);
        sem_wait(&mutex_cpu_vacia);
		
		//log_info(logger,"CORTO PLAZO");

		log_info(logger, "Hay procesos en READY y la CPU esta vacia, PLANIFICO.");
		//estadoExec = malloc(sizeof(pcb)); o hacer free y malloc

		if (strcmp(config->algoritmo_planificacion,"FIFO") == 0) {
			sem_wait(&sem_exec);
			estadoExec = calcular_FIFO();
			sem_post(&sem_exec);
		}
		else {
			sem_wait(&sem_exec);
			estadoExec = calcularSRT();
			sem_post(&sem_exec);
		}
		sem_post(&quieroIrAExec);
		//log_error(logger,"FIN CORTO PLAZO");
    }
}