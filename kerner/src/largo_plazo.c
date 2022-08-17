#include "largo_plazo.h"


void* execAExit() {
	while(1) {
		sem_wait(&meQuieroIr);
		pcb* pcbFinalizadaCpu/* = malloc(sizeof(pcb))*/;

		sem_wait(&sem_exit);
		pcbFinalizadaCpu = list_remove(estadoExit,0);
		sem_post(&sem_exit);

		log_info(logger, "Mande la PCB %d a EXIT.", pcbFinalizadaCpu->id);

		sacarPCBDeMemoria(pcbFinalizadaCpu); // esta hecho como un char* y un uint32_t
		enviarMensajeDeFinalizacion(pcbFinalizadaCpu);

		
		list_destroy_and_destroy_elements(pcbFinalizadaCpu -> listaInstrucciones, free);
	}
	return NULL;
}

void* newAReady() {
	while(1) {
		sem_wait(&procesosNewReady);
		pcb* pcbNew/* = malloc(sizeof(pcb))*/;
		sem_wait(&sem_new);
		pcbNew = list_remove(estadoNew,0);
		sem_post(&sem_new);

		log_info(logger, "Moviendo la PCB %d de NEW a READY.", pcbNew->id);

		pcbNew -> estado_proceso = READY;
		agregarAMemoria(pcbNew); // JUAMPI
		
		sem_wait(&mutex_mandarAReady);
		mandarAReady(pcbNew);
		sem_post(&mutex_mandarAReady);

	}
	return NULL;
}