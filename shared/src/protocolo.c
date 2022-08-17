#include "../include/protocolo.h"

static void* serializar_mensaje_memoria(size_t* size, char* mensaje, uint32_t referencia) {
    size_t size_mensaje = strlen(mensaje) + 1;
    *size =
          sizeof(size_t)    
        + sizeof(size_t)    
        + size_mensaje         
        + sizeof(uint32_t);  
    size_t size_payload = *size - sizeof(size_t);

    void* stream = malloc(*size);

    memcpy(stream, &size_payload, sizeof(size_t));
    memcpy(stream+sizeof(size_t), &size_mensaje, sizeof(size_t));
    memcpy(stream+sizeof(size_t)*2, mensaje, size_mensaje);
    memcpy(stream+sizeof(size_t)*2+size_mensaje, &referencia, sizeof(uint32_t));

    return stream;
}

static void deserializar_mensaje_memoria(void* stream, char** mensaje, uint32_t* referencia) {
    
    size_t size_mensaje;
    memcpy(&size_mensaje, stream, sizeof(size_t));

    char* r_mensaje = malloc(size_mensaje);
    memcpy(r_mensaje, stream+sizeof(size_t), size_mensaje);
    *mensaje = r_mensaje;

    
    memcpy(referencia, stream+sizeof(size_t)+size_mensaje, sizeof(uint32_t));
}

bool send_mensaje_memoria(int fd, char* mensaje, uint32_t referencia) {
    size_t size;
    void* stream = serializar_mensaje_memoria(&size, mensaje, referencia);
    if (send(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mensaje_memoria(int fd, char** mensaje, uint32_t* referencia) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
        return false;

    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_WAITALL) != size_payload) {
        free(stream);
        return false;
    }

    deserializar_mensaje_memoria(stream, mensaje, referencia);

    free(stream);
    return true;
}


static void* serializar_dos_uint(uint32_t numero1, uint32_t numero2) {
    void* stream = malloc(sizeof(op_code) + sizeof(uint32_t) * 2);

    memcpy(stream, &numero1, sizeof(uint32_t));
    memcpy(stream+sizeof(uint32_t), &numero2, sizeof(uint32_t));
    return stream;
}

static void deserializar_dos_uint(void* stream, uint32_t* numero1, uint32_t* numero2) {
    memcpy(numero1, stream, sizeof(uint32_t));
    memcpy(numero2, stream+sizeof(uint32_t), sizeof(uint32_t));
}

bool send_dos_uint(int fd, uint32_t numero1, uint32_t numero2) {
    size_t size = sizeof(uint32_t) * 2;
    void* stream = serializar_dos_uint(numero1, numero2);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_dos_uint(int fd, uint32_t* numero1, uint32_t* numero2) {
    size_t size = sizeof(uint32_t) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_dos_uint(stream, numero1, numero2);

    free(stream);
    return true;
}





static void* serializar_lista_instrucciones(t_list* lista, uint32_t tamanioProceso) {	
	// calculo tamaños
	size_t tamanio_op_code = sizeof(op_code);
	size_t tamanio_parametro = sizeof(uint32_t);
	size_t tamanio_instruccion = tamanio_op_code + 2*tamanio_parametro;
	size_t tamanio_stream = sizeof(uint32_t) + tamanio_instruccion * list_size(lista);

	//printf("LIST_SIZE(LISTA) = %d\n", tamanio_stream);
	
	// armo el stream
	void* stream = malloc(tamanio_stream);
	
	// serializo el tamaño del proceso
	memcpy(stream, &tamanioProceso, sizeof(uint32_t));
	
	// serializo las instrucciones
	for (int i=0; i < list_size(lista); i++) {
		//leo la instrucción
		instruccion* leido = malloc(sizeof(instruccion));
		leido = list_get(lista, i);
		// convierto el cop en enum
		op_code cop = generarCode(leido -> id_INSTRUCCION);
		
		// meto cop en el stream
		memcpy(stream + sizeof(uint32_t) + i*(tamanio_op_code + 2*tamanio_parametro), &cop, tamanio_op_code);
		// leo los parámetros
		uint32_t param1 = leido -> param[0];
		uint32_t param2 = leido -> param[1];
		
		memcpy(stream + sizeof(uint32_t) + tamanio_op_code + i*(tamanio_op_code + 2*tamanio_parametro), &param1, tamanio_parametro);
		memcpy(stream + sizeof(uint32_t) + tamanio_op_code + tamanio_parametro + i*(tamanio_op_code + 2*tamanio_parametro), &param2, tamanio_parametro);
		
	}
	return stream;
}

bool send_lista_instrucciones(int conexion, t_list* lista, uint32_t tamanioProceso) {
	// calculo tamaños
	size_t tamanio_instruccion = sizeof(op_code) + 2*sizeof(uint32_t);
	size_t tamanio = sizeof(uint32_t) + tamanio_instruccion * list_size(lista);
	
	// serializo la lista
	void* stream = serializar_lista_instrucciones(lista, tamanioProceso);

	// mando la lista
	if (send(conexion, stream, tamanio, 0) != tamanio){
		free(stream);
		return false;
	}
	free(stream);
	return true;
}

static void deserializar_un_parametro(void* stream, uint32_t* param) {
	memcpy(param, stream, sizeof(uint32_t));
}

static void deserializar_dos_parametros(void* stream, uint32_t* param1, uint32_t* param2) {
	memcpy(param1, stream, sizeof(uint32_t));
	memcpy(param2, stream + sizeof(uint32_t), sizeof(uint32_t));
}

bool recv_cero_parametros(int conexion) {
	// saco la basura
	void* stream = malloc(2 * sizeof(uint32_t));
	if (recv(conexion, stream, 2 * sizeof(uint32_t), MSG_WAITALL) != 2 * sizeof(uint32_t)) {
		free(stream);
		return false;
	}
	free(stream);
	return true;
}

bool recv_un_parametro(int conexion, uint32_t* param) {
	size_t tamanio = sizeof(uint32_t);
	void* stream = malloc(tamanio);
	
	// recibo el parámetro
	
	if (recv(conexion, stream, tamanio, MSG_WAITALL) != tamanio){
		free(stream);
		return false;
	}
	deserializar_un_parametro(stream, param);
	free(stream);
	
	// saco la basura
	
	stream = malloc(tamanio);
	if (recv(conexion, stream, tamanio, MSG_WAITALL) != tamanio) {
		free(stream);
		return false;
	}
	free(stream);
	return true;
}

bool recv_dos_parametros(int conexion, uint32_t* param1, uint32_t* param2) {
	size_t tamanio = sizeof(uint32_t) * 2;
	void* stream = malloc(tamanio);
	
	if (recv(conexion, stream, tamanio, MSG_WAITALL) != tamanio){
		free(stream);
		return false;
	}
	deserializar_dos_parametros(stream, param1, param2);
	free(stream);
	return true;
}

static void* serializar_PCB (pcb* pcbASerializar, size_t* tamanio_stream) {
	
	uint32_t id = pcbASerializar -> id;
	uint32_t tamProceso = pcbASerializar -> tamanio_bytes;
	t_list* listaInstrucciones = pcbASerializar -> listaInstrucciones;
	estado estadoProceso = pcbASerializar -> estado_proceso;
	uint32_t programCounter = pcbASerializar -> program_counter;
	uint32_t tablaPaginas = pcbASerializar -> tabla_paginas; // ver qué nombre le pone Juan
	uint32_t estimacionRafaga = pcbASerializar -> estimacion_rafaga;

	size_t tamanio_id = sizeof(uint32_t);
	size_t tamanio_tamProceso = sizeof(uint32_t);
	size_t tamanio_tamLista = sizeof(size_t);
	size_t tamLista = list_size(listaInstrucciones) * sizeof(instruccion);
	size_t tamanio_estado = sizeof(estado);
	size_t tamanio_PC = sizeof(uint32_t);
	size_t tamanio_tablaPaginas = sizeof(uint32_t);
	size_t tamanio_estimacionRafaga = sizeof(uint32_t);
	size_t tamanio_tamPayload = sizeof(size_t);
	
	size_t tamPayload = tamanio_id + tamanio_tamProceso + tamanio_tamLista + tamLista +
						tamanio_estado + tamanio_PC + tamanio_tablaPaginas +
						tamanio_estimacionRafaga;
	*tamanio_stream = tamPayload + tamanio_tamPayload;
	
	// creo el stream
	void* stream = malloc(*tamanio_stream);
	instruccion* instAGuardar;
	int i=0;
	op_code cop;			
	uint32_t param0;		
	uint32_t param1;		

	
	memcpy(stream, &tamPayload, tamanio_tamPayload);
	
	
	
	memcpy(stream + tamanio_tamPayload, &id, tamanio_id);
	memcpy(stream + tamanio_tamPayload + tamanio_id, &tamProceso, tamanio_tamProceso);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso, &tamLista, tamanio_tamLista);
	while (i < list_size(listaInstrucciones)) {
		instAGuardar = list_get(listaInstrucciones,i);

		

		cop = generarCode(instAGuardar -> id_INSTRUCCION);
		param0 = (instAGuardar -> param[0]);
		param1 = instAGuardar -> param[1];
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)), &cop, sizeof(op_code));
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)) + sizeof(op_code), &param0, sizeof(uint32_t));
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)) + sizeof(op_code) + sizeof(uint32_t), &param1, sizeof(uint32_t));
		i++;
	}
	
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion), &estadoProceso, tamanio_estado);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado, &programCounter, tamanio_PC);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado + tamanio_PC, &tablaPaginas, tamanio_tablaPaginas);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado + tamanio_PC + tamanio_tablaPaginas, &estimacionRafaga, tamanio_estimacionRafaga);
		
	return stream;
}

bool send_PCB (int conexion, pcb* pcbASerializar) {
	size_t tamanio_stream; 
	void* stream = serializar_PCB(pcbASerializar, &tamanio_stream);
	
	if (send(conexion, stream, tamanio_stream, 0) != tamanio_stream) {
		free(stream);
		return false;
	}
	free(stream);
	return true;
}

static void deserializar_PCB(void* payload, pcb* pcbRecibida) {
	// armo variables auxiliares donde guardar lo recibido
	uint32_t id;
	uint32_t tamProceso;
	size_t tamLista;
	t_list* listaInstrucciones;
	estado estadoProceso;
	uint32_t programCounter;
	uint32_t tablaPaginas;
	uint32_t estimacionRafaga;
	
	// guardo lo recibido en las variables auxiliares (obviamente)
	memcpy(&id, payload, sizeof(uint32_t));
	memcpy(&tamProceso, payload + sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&tamLista, payload + 2*sizeof(uint32_t), sizeof(size_t));
	
	// abro paréntesis: aloco memoria para la lista
	//listaInstrucciones = malloc(tamLista);
	// cierro paréntesis
	int i = 0;
	listaInstrucciones = list_create();
	op_code cop;
	uint32_t param0;
	uint32_t param1;
	while(cop != EXIT) {
		instruccion* nuevaInst = malloc(sizeof(instruccion));
		memcpy(&cop,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t)),sizeof(op_code));
		memcpy(&param0,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t))+sizeof(op_code),sizeof(uint32_t));
		memcpy(&param1,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t))+sizeof(op_code)+sizeof(uint32_t),sizeof(uint32_t));
		nuevaInst -> id_INSTRUCCION = generarString(cop);
		nuevaInst -> param[0] = param0;
		nuevaInst -> param[1] = param1;
		list_add(listaInstrucciones, nuevaInst);
		//printf("Lei el cop %s\n",generarString(cop));
		//printf("Lei el parametro%d\n",param1);
		//printf("Aguregue la instruccion %d\n", i);
		//printf("Lei el parametro %d\n",param0);
		i++;
	}
	//memcpy(listaInstrucciones, payload + 2*sizeof(uint32_t) + sizeof(size_t), tamLista);
	memcpy(&estadoProceso, payload + 2*sizeof(uint32_t) + sizeof(size_t) + tamLista, sizeof(estado));
	memcpy(&programCounter, payload + 2*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));
	memcpy(&tablaPaginas, payload + 3*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));
	memcpy(&estimacionRafaga, payload + 4*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));

	// lo guardo en la PCB

	pcbRecibida -> id = id;
	pcbRecibida -> tamanio_bytes = tamProceso;
	pcbRecibida -> listaInstrucciones = listaInstrucciones;
	pcbRecibida -> estado_proceso = estadoProceso;
	pcbRecibida -> program_counter = programCounter;
	pcbRecibida -> tabla_paginas = tablaPaginas;
	pcbRecibida -> estimacion_rafaga = estimacionRafaga;

	//free(listaInstrucciones);
}

bool recv_PCB (int conexion, pcb* pcbRecibida) {
	// recibo el tamaño del payload
	size_t tamanio_payload;
	if (recv(conexion, &tamanio_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
		return false;

	// recibo el payload
	void* stream_payload = malloc(tamanio_payload);
	if (recv(conexion, stream_payload, tamanio_payload, MSG_WAITALL) != tamanio_payload) {
		free(stream_payload);
		return false;
	}
	
	deserializar_PCB(stream_payload, pcbRecibida);
	
	// PRUEBAS

	free(stream_payload);
	return true;
}


static void* serializar_PCB_mas_int (pcb* pcbASerializar, size_t* tamanio_stream, uint32_t* elInt) {
	// leo PCB
	uint32_t id = pcbASerializar -> id;
	uint32_t tamProceso = pcbASerializar -> tamanio_bytes;
	t_list* listaInstrucciones = pcbASerializar -> listaInstrucciones;
	estado estadoProceso = pcbASerializar -> estado_proceso;
	uint32_t programCounter = pcbASerializar -> program_counter;
	uint32_t tablaPaginas = pcbASerializar -> tabla_paginas;
	uint32_t estimacionRafaga = pcbASerializar -> estimacion_rafaga;

	// calculo tamaños
	size_t tamanio_id = sizeof(uint32_t);
	size_t tamanio_tamProceso = sizeof(uint32_t);
	size_t tamanio_tamLista = sizeof(size_t);
	size_t tamLista = list_size(listaInstrucciones) * sizeof(instruccion);
	size_t tamanio_estado = sizeof(estado);
	size_t tamanio_PC = sizeof(uint32_t);
	size_t tamanio_tablaPaginas = sizeof(uint32_t);
	size_t tamanio_estimacionRafaga = sizeof(uint32_t);
	size_t tamanio_tamPayload = sizeof(size_t);
	size_t tamanio_elInt = sizeof(uint32_t);

	//printf("El estado en serializar es %s\n", generar_string_de_estado(estadoProceso));
	
	size_t tamPayload = tamanio_id + tamanio_tamProceso + tamanio_tamLista + tamLista +
						tamanio_estado + tamanio_PC + tamanio_tablaPaginas +
						tamanio_estimacionRafaga + tamanio_elInt;
	*tamanio_stream = tamPayload + tamanio_tamPayload;
	
	// creo el stream
	void* stream = malloc(*tamanio_stream);
	instruccion* instAGuardar;
	int i=0;
	op_code cop;			//HACER TODOS
	uint32_t param0;		//LOS FREE
	uint32_t param1;		//CUANDO TERMINA LA PCB

	// serializo el tamaño del payload
	memcpy(stream, &tamPayload, tamanio_tamPayload);
	
	// serializo el payload en si
	memcpy(stream + tamanio_tamPayload, &id, tamanio_id);
	memcpy(stream + tamanio_tamPayload + tamanio_id, &tamProceso, tamanio_tamProceso);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso, &tamLista, tamanio_tamLista);
	while (i < list_size(listaInstrucciones)) {
		instAGuardar = list_get(listaInstrucciones,i);
		cop = generarCode(instAGuardar -> id_INSTRUCCION);
		param0 = (instAGuardar -> param[0]);
		param1 = instAGuardar -> param[1];
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)), &cop, sizeof(op_code));
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)) + sizeof(op_code), &param0, sizeof(uint32_t));
		memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*(sizeof(instruccion)) + sizeof(op_code) + sizeof(uint32_t), &param1, sizeof(uint32_t));
		i++;
	}
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion), &estadoProceso, tamanio_estado);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado, &programCounter, tamanio_PC);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado + tamanio_PC, &tablaPaginas, tamanio_tablaPaginas);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado + tamanio_PC + tamanio_tablaPaginas, &estimacionRafaga, tamanio_estimacionRafaga);
	memcpy(stream + tamanio_tamPayload + tamanio_id + tamanio_tamProceso + tamanio_tamLista + i*sizeof(instruccion) + tamanio_estado + tamanio_PC + tamanio_tablaPaginas + tamanio_estimacionRafaga, elInt, tamanio_elInt);
	
	return stream;
}

static uint32_t deserializar_PCB_mas_int(void* payload, pcb* pcbRecibida) {
	// armo variables auxiliares donde guardar lo recibido
	uint32_t id;
	uint32_t tamProceso;
	size_t tamLista;
	t_list* listaInstrucciones;
	estado estadoProceso;
	uint32_t programCounter;
	uint32_t tablaPaginas;
	uint32_t estimacionRafaga;
	uint32_t elInt;
	
	// guardo lo recibido en las variables auxiliares (obviamente)
	memcpy(&id, payload, sizeof(uint32_t));
	memcpy(&tamProceso, payload + sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&tamLista, payload + 2*sizeof(uint32_t), sizeof(size_t));
	
	// abro paréntesis: aloco memoria para la lista
	//listaInstrucciones = malloc(tamLista);
	// cierro paréntesis
	int i = 0;
	listaInstrucciones = list_create();
	op_code cop;
	uint32_t param0;
	uint32_t param1;
	while(cop != EXIT) {
		instruccion* nuevaInst = malloc(sizeof(instruccion));
		memcpy(&cop,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t)),sizeof(op_code));
		memcpy(&param0,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t))+sizeof(op_code),sizeof(uint32_t));
		memcpy(&param1,payload + 2*sizeof(uint32_t) + sizeof(size_t)+i*(sizeof(op_code)+2*sizeof(uint32_t))+sizeof(op_code)+sizeof(uint32_t),sizeof(uint32_t));
		nuevaInst -> id_INSTRUCCION = generarString(cop);
		nuevaInst -> param[0] = param0;
		nuevaInst -> param[1] = param1;
		list_add(listaInstrucciones, nuevaInst);
		
		i++;
	}
	//memcpy(listaInstrucciones, payload + 2*sizeof(uint32_t) + sizeof(size_t), tamLista);
	memcpy(&estadoProceso, payload + 2*sizeof(uint32_t) + sizeof(size_t) + tamLista, sizeof(estado));
	memcpy(&programCounter, payload + 2*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));
	memcpy(&tablaPaginas, payload + 3*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));
	memcpy(&estimacionRafaga, payload + 4*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));
	memcpy(&elInt, payload + 5*sizeof(uint32_t) + sizeof(size_t) + tamLista + sizeof(estado), sizeof(uint32_t));

	// lo guardo en la PCB

	pcbRecibida -> id = id;
	pcbRecibida -> tamanio_bytes = tamProceso;
	pcbRecibida -> listaInstrucciones = listaInstrucciones;
	pcbRecibida -> estado_proceso = estadoProceso;
	pcbRecibida -> program_counter = programCounter;
	pcbRecibida -> tabla_paginas = tablaPaginas;
	pcbRecibida -> estimacion_rafaga = estimacionRafaga;


	return elInt;
}

uint32_t recv_PCB_mas_int (int conexion, pcb* pcbRecibida) {
	// recibo el tamaño del payload
	size_t tamanio_payload;
	if (recv(conexion, &tamanio_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
		return -1; // ATENCIÓN: ESTE ES EL CÓDIGO DE ERROR

	// recibo el payload
	void* stream_payload = malloc(tamanio_payload);
	if (recv(conexion, stream_payload, tamanio_payload, MSG_WAITALL) != tamanio_payload) {
		free(stream_payload);
		return -1; // ATENCIÓN: ESTE ES EL CÓDIGO DE ERROR
	}
	
	uint32_t elInt = deserializar_PCB_mas_int(stream_payload, pcbRecibida);

	//printf("En el recv, el estado del proceso es %s\n", generar_string_de_estado(pcbRecibida->estado_proceso));

	free(stream_payload);
	return elInt;
}

bool send_PCB_mas_int (int conexion, pcb* pcbASerializar, uint32_t elInt) {
	size_t tamanio_stream; 
	void* stream = serializar_PCB_mas_int(pcbASerializar, &tamanio_stream, &elInt);
	
	if (send(conexion, stream, tamanio_stream, 0) != tamanio_stream) {
		free(stream);
		printf("\n\n\n(send_pcb_mas_int) NO ENVIADO (SOCKET %d)\n\n\n", conexion);
		return false;
	}
	
	free(stream);
	return true;
}



static void* serializar_mensaje_consola(size_t* size, char* mensaje) {
    size_t size_mensaje = strlen(mensaje) + 1;
    *size =
          sizeof(size_t)    // total
        + sizeof(size_t)    // size de char* peli
        + size_mensaje;        // char* peli
    size_t size_payload = *size - sizeof(size_t);

    void* stream = malloc(*size);

    memcpy(stream, &size_payload, sizeof(size_t));
    memcpy(stream+sizeof(size_t), &size_mensaje, sizeof(size_t));
    memcpy(stream+sizeof(size_t)*2, mensaje, size_mensaje);

    return stream;
}

static void deserializar_mensaje_consola(void* stream, char** mensaje) {
    // Peli
    size_t size_mensaje;
    memcpy(&size_mensaje, stream, sizeof(size_t));

    char* r_mensaje = malloc(size_mensaje);
    memcpy(r_mensaje, stream+sizeof(size_t), size_mensaje);
    *mensaje = r_mensaje;

}

bool send_mensaje_consola(int fd, char* mensaje) {
    size_t size;
    void* stream = serializar_mensaje_consola(&size, mensaje);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mensaje_consola(int fd, char** mensaje) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
        return false;

    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_WAITALL) != size_payload) {
        free(stream);
        return false;
    }

    deserializar_mensaje_consola(stream, mensaje);

    free(stream);
    return true;
}

///////////////////////////////////////////////////////////

static void* serializar_mensaje_interrupcion(size_t* size, char* mensaje) {
    size_t size_mensaje = strlen(mensaje) + 1;
    *size =
          sizeof(size_t)    // total
        + sizeof(size_t)    // size de char* peli
        + size_mensaje;        // char* peli
    size_t size_payload = *size - sizeof(size_t);

    void* stream = malloc(*size);

    memcpy(stream, &size_payload, sizeof(size_t));
    memcpy(stream+sizeof(size_t), &size_mensaje, sizeof(size_t));
    memcpy(stream+sizeof(size_t)*2, mensaje, size_mensaje);

    return stream;
}

static void deserializar_mensaje_interrupcion(void* stream, char** mensaje) {
    // Peli
    size_t size_mensaje;
    memcpy(&size_mensaje, stream, sizeof(size_t));

    char* r_mensaje = malloc(size_mensaje);
    memcpy(r_mensaje, stream+sizeof(size_t), size_mensaje);
    *mensaje = r_mensaje;

}

bool send_mensaje_interrupcion(int fd, char* mensaje) {
    size_t size;
    void* stream = serializar_mensaje_interrupcion(&size, mensaje);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mensaje_interrupcion(int fd, char** mensaje) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_DONTWAIT) != sizeof(size_t))
        return false;

    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_DONTWAIT) != size_payload) {
        free(stream);
        return false;
    }

    deserializar_mensaje_interrupcion(stream, mensaje);

    free(stream);
    return true;
}

static void* serializar_char_y_dos_int(size_t* size, char* mensaje, uint32_t referencia1,uint32_t referencia2) {
    size_t size_mensaje = strlen(mensaje) + 1;
    *size =
          sizeof(size_t)    // total
        + sizeof(size_t)    // size de char* peli
        + size_mensaje         // char* peli
        + sizeof(uint32_t)  // cant_pochoclos
		+ sizeof(uint32_t);
    size_t size_payload = *size - sizeof(size_t);

    void* stream = malloc(*size);

    memcpy(stream, &size_payload, sizeof(size_t));
    memcpy(stream+sizeof(size_t), &size_mensaje, sizeof(size_t));
    memcpy(stream+sizeof(size_t)*2, mensaje, size_mensaje);
    memcpy(stream+sizeof(size_t)*2+size_mensaje, &referencia1, sizeof(uint32_t));
	memcpy(stream+sizeof(size_t)*2+size_mensaje + sizeof(uint32_t), &referencia2, sizeof(uint32_t));

    return stream;
}

static void deserializar_char_y_dos_int(void* stream, char** mensaje, uint32_t* referencia1, uint32_t* referencia2) {
    // Peli
    size_t size_mensaje;
    memcpy(&size_mensaje, stream, sizeof(size_t));

    char* r_mensaje = malloc(size_mensaje);
    memcpy(r_mensaje, stream+sizeof(size_t), size_mensaje);
    *mensaje = r_mensaje;

    // Pochoclos
    memcpy(referencia1, stream+sizeof(size_t)+size_mensaje, sizeof(uint32_t));
	memcpy(referencia2, stream+sizeof(size_t)+size_mensaje + sizeof(uint32_t), sizeof(uint32_t));
}

bool send__char_y_dos_int(int fd, char* mensaje, uint32_t referencia1, uint32_t referencia2) {
    size_t size;
    void* stream = serializar_char_y_dos_int(&size, mensaje, referencia1,referencia2);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv__char_y_dos_int(int fd, char** mensaje, uint32_t* referencia1,uint32_t* referencia2) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
        return false;

    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_WAITALL) != size_payload) {
        free(stream);
        return false;
    }

    deserializar_char_y_dos_int(stream, mensaje, referencia1, referencia2);

    free(stream);
    return true;
}


static void* serializar_char_y_tres_int(size_t* size, char* mensaje, uint32_t referencia1,uint32_t referencia2,uint32_t referencia3) {
    size_t size_mensaje = strlen(mensaje) + 1;
    *size =
          sizeof(size_t)    // total
        + sizeof(size_t)    // size de char* peli
        + size_mensaje         // char* peli
        + sizeof(uint32_t)  // cant_pochoclos
		+ sizeof(uint32_t)
		+ sizeof(uint32_t);
		
    size_t size_payload = *size - sizeof(size_t);

    void* stream = malloc(*size);

    memcpy(stream, &size_payload, sizeof(size_t));
    memcpy(stream+sizeof(size_t), &size_mensaje, sizeof(size_t));
    memcpy(stream+sizeof(size_t)*2, mensaje, size_mensaje);
    memcpy(stream+sizeof(size_t)*2+size_mensaje, &referencia1, sizeof(uint32_t));
	memcpy(stream+sizeof(size_t)*2+size_mensaje + sizeof(uint32_t), &referencia2, sizeof(uint32_t));
	memcpy(stream+sizeof(size_t)*2+size_mensaje + sizeof(uint32_t) + sizeof(uint32_t), &referencia3, sizeof(uint32_t));

    return stream;
}

static void deserializar_char_y_tres_int(void* stream, char** mensaje, uint32_t* referencia1, uint32_t* referencia2, uint32_t* referencia3) {
    // Peli
    size_t size_mensaje;
    memcpy(&size_mensaje, stream, sizeof(size_t));

    char* r_mensaje = malloc(size_mensaje);
    memcpy(r_mensaje, stream+sizeof(size_t), size_mensaje);
    *mensaje = r_mensaje;

    // Pochoclos
    memcpy(referencia1, stream+sizeof(size_t)+size_mensaje, sizeof(uint32_t));
	memcpy(referencia2, stream+sizeof(size_t)+size_mensaje + sizeof(uint32_t), sizeof(uint32_t));
	memcpy(referencia3, stream+sizeof(size_t)+size_mensaje + sizeof(uint32_t) + sizeof(uint32_t), sizeof(uint32_t));
}

bool send_char_y_tres_int(int fd, char* mensaje, uint32_t referencia1, uint32_t referencia2,uint32_t referencia3) {
    size_t size;
    void* stream = serializar_char_y_tres_int(&size, mensaje, referencia1,referencia2,referencia3);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_char_y_tres_int(int fd, char** mensaje, uint32_t* referencia1,uint32_t* referencia2,uint32_t* referencia3) {
    size_t size_payload;
    if (recv(fd, &size_payload, sizeof(size_t), MSG_WAITALL) != sizeof(size_t))
        return false;

    void* stream = malloc(size_payload);
    if (recv(fd, stream, size_payload, MSG_WAITALL) != size_payload) {
        free(stream);
        return false;
    }

    deserializar_char_y_tres_int(stream, mensaje, referencia1, referencia2,referencia3);

    free(stream);
    return true;
}
