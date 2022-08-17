#include "mediano_plazo.h"


void* suspendedReady_Ready() {
	while(1) {
		sem_wait(&procesosSuspReadyReady);
		//log_info(logger, "Ejecuto suspendedReady_Ready");

		pcb* pcbNew;
		sem_wait(&sem_suspended_ready);
		pcbNew = list_remove(estadoSuspendedReady,0);
		sem_post(&sem_suspended_ready);
		pcbNew -> estado_proceso = READY;
		agregarAMemoria(pcbNew); 
		
		sem_wait(&mutex_mandarAReady);
		mandarAReady(pcbNew);								
		sem_post(&mutex_mandarAReady);

		log_info(logger, "Mande la PCB %d de SUSPENDED READY a READY", pcbNew->id);
	}

	return NULL;
}




void* suspended_blocked_Suspended_ready() {
	while(1) {
		sem_wait(&procesosSuspBlockedSuspReady);

		
		
		pcbIO* pcb_io_bloqueada;
		pcb* pcb_bloqueada;
		


		sem_wait(&sem_blocked);
		pcb_io_bloqueada = list_remove(estadoBlocked,0); 
		sem_post(&sem_blocked);
		sem_post(&listaBloqueadosAlDia);

		pcb_bloqueada = pcb_io_bloqueada -> pcbBloqueada;
		
		//log_info(logger, "En blocked_Ready, el ID de la PCB es %d", pcb_bloqueada->id);
		
		pcb_bloqueada -> estado_proceso = SUSPENDED_READY;
		sem_post(&mutex_io);
		sem_wait(&sem_suspended_ready);
		list_add(estadoSuspendedReady,pcb_bloqueada);
		sem_post(&sem_suspended_ready);
		sem_post(&quieroIrAMemoria);

		log_info(logger, "Mande la PCB %d de SUSPENDED BLOCKED a SUSPENDED READY.", pcb_bloqueada -> id);

	}
	return NULL;
}