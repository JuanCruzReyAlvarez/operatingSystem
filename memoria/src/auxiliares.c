#include "auxiliares.h"

uint32_t ceilCasero(float numero) {
    uint32_t ceil = (uint32_t)numero;
    return (ceil < numero) ? ceil + 1 : ceil;
}

uint32_t floorCasero(float numero) {
    uint32_t floor = (uint32_t)numero;
    return (floor > numero) ? floor - 1 : floor;
}

uint32_t minimoEntre(uint32_t a, uint32_t b) {
    return a<b ? a : b;
}

void config_get_string_values() {

    config = malloc(sizeof(datosConfig));

    t_config* nuevo_config = config_create("memoria.config"); // "cfg/kernel.config"

	
	char* puerto_escucha = config_get_string_value(nuevo_config,"PUERTO_ESCUCHA");
    char* tam_memoria = config_get_string_value(nuevo_config,"TAM_MEMORIA");
    char* tam_pagina = config_get_string_value(nuevo_config,"TAM_PAGINA");
    char* entradas_por_tabla = config_get_string_value(nuevo_config,"ENTRADAS_POR_TABLA");
    char* retardo_memoria = config_get_string_value(nuevo_config,"RETARDO_MEMORIA");
    char* algoritmo_reemplazo = config_get_string_value(nuevo_config,"ALGORITMO_REEMPLAZO");
    char* marcos_por_proceso = config_get_string_value(nuevo_config,"MARCOS_POR_PROCESO");
    char* retardo_swap = config_get_string_value(nuevo_config,"RETARDO_SWAP");
    char* path_swap = config_get_string_value(nuevo_config,"PATH_SWAP");
    
    config -> puerto_escucha = puerto_escucha;
    config -> tam_memoria = (uint32_t)atoi(tam_memoria);
    config -> tam_pagina = (uint32_t)atoi(tam_pagina);
    config -> entradas_por_tabla = (uint32_t)atoi(entradas_por_tabla);
    config -> retardo_memoria = (uint32_t)atoi(retardo_memoria);
    config -> algoritmo_reemplazo = algoritmo_reemplazo;
    config -> marcos_por_proceso = (uint32_t)atoi(marcos_por_proceso);
    config -> retardo_swap = (uint32_t)atoi(retardo_swap);
    config -> path_swap = path_swap;

    //config_destroy(nuevo_config);

    log_info(logger, "Config leida.");   
}

void init_semaforos() {
    sem_init(&mutex_primerNivel,        0,  1);
    sem_init(&mutex_segundoNivel,       0,  1);
    sem_init(&sem_inicializarProceso,   0,  0);
    sem_init(&sem_suspenderProceso,     0,  0);
    sem_init(&sem_finalizarProceso,     0,  0);
    sem_init(&mutex_tablaMarcos,        0,  1);
    sem_init(&mutex_SWAP,               0,  1);
    sem_init(&swapListo,                0,  1);
    sem_init(&sem_handshakeCPU,		0,  0);
    sem_init(&sem_handshakeKerner,	0,  0);
}

void init_listas() {
    tablasDePrimerNivel = list_create();
    tablasDeSegundoNivel = list_create();
}

void init_espacioUsuario() { espacioUsuario = malloc(config-> tam_memoria); }

void init_tablaMarcos() {

    tablaMarcos = dictionary_create();
    uint32_t cantMarcos = floorCasero (config->tam_memoria/config->tam_pagina);

    for (int i =0; i<cantMarcos; i++){
        valueTablaMarcos* value = malloc(sizeof(value));
        value -> pid = -1;
        value ->libre = true;
        dictionary_put(tablaMarcos,string_itoa(i),value);
        
    }
}



uint32_t tamanioArchivo(uint32_t tamProceso) {      // el tamaño que tiene que tener el archivo?
    return ((ceilCasero(tamProceso/(config -> tam_pagina))) * (config -> tam_pagina));
}

uint32_t tamanioActualDelArchivo(FILE* archivoAbierto) {    // en bytes
    fseek(archivoAbierto, 0, SEEK_END);
    uint32_t tam = ftell(archivoAbierto);
    fseek(archivoAbierto, 0, SEEK_SET);
    return tam;
}

void esperarSwap() {
    sem_wait(&swapListo);
    log_warning(logger,"\n\nACCEDO A SWAP\n\n");
    usleep((config -> retardo_swap) * 1000);
}


////////////////////////////////////RESPUESTAS A KERNER <3//////////////////////////////////


/////////////////////////////////////////////INICIALIZAR PROCESO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* generarNombreDeArchivo(uint32_t pid) {
    char* nombre = string_new();
    string_append(&nombre, string_itoa(pid));
    string_append(&nombre, ".swap");
    return nombre;
}

char* generarPath(uint32_t pid) {
    char* path = string_new();

    string_append(&path, config -> path_swap);
    string_append(&path, "/");

    char* nombreArchivo = generarNombreDeArchivo(pid);

    string_append(&path, nombreArchivo);

    return path;
}

void crearArchivo(uint32_t pid, uint32_t tamProceso) {
    //uint32_t sizeArchivo = (tamProceso);
    char* path = generarPath(pid);
    void* espacioSWAP = malloc(tamProceso);
    sem_wait(&mutex_SWAP);
    esperarSwap();
    sem_post(&mutex_SWAP);

    FILE* archivo = fopen(path, "wb");
    fwrite(espacioSWAP, tamProceso, 1, archivo);
    fclose(archivo);
    log_info(logger, "Archivo generado. Path: %s", path);
    free(espacioSWAP);
    sem_post(&swapListo);
}

void asignarMarcos(uint32_t pidProceso) {
    
    int j = 0;
    for(int i =0; i< dictionary_size(tablaMarcos); i++){
        valueTablaMarcos* value = dictionary_get(tablaMarcos,string_itoa(i));
        if((value->pid) == -1){
            for(j = 0; j< config->marcos_por_proceso; j++){
                value = dictionary_get(tablaMarcos,string_itoa(i+j));
                value->pid = pidProceso;
                value->libre = true;
                //log_error(logger, "LE ASIGNÉ EL MARCO %d AL PID %d", i+j, value -> pid);
            }
            log_info(logger, "se asignaron %d marcos al pid %d", j, pidProceso);
            return ;
        }
    }
    if (j==0) {log_error(logger,"No se pudo asignar un marco");}
    
}

uint32_t generarEstructuras(uint32_t pid, uint32_t tamProceso) {

    uint32_t cantPaginas = ceilCasero(tamProceso / (config -> tam_pagina)); // no estamos chequeando que se puedan direccionar todas, se supone que no va a pasar

    sem_wait(&mutex_tablaMarcos);
    asignarMarcos(pid);
    sem_post(&mutex_tablaMarcos);

    int entradasPrimerNivel = 0;
    int paginasCreadas = 0;
    t_dictionary* tablaPrimerNivel = malloc(sizeof(t_dictionary));
    tablaPrimerNivel = dictionary_create();
    while(entradasPrimerNivel < (config -> entradas_por_tabla) && paginasCreadas < cantPaginas) {
        
        int entradasSegundoNivel = 0;
        t_dictionary* tablaSegundoNivel = malloc(sizeof(t_dictionary));
        tablaSegundoNivel = dictionary_create();

        while (entradasSegundoNivel < (config -> entradas_por_tabla) && paginasCreadas < cantPaginas) {
            datosPagina* data = malloc(sizeof(datosPagina));
            data -> marco = -1;
            data -> presencia = false;
            data -> modificado = false;
            data -> uso = false;

            dictionary_put(tablaSegundoNivel, string_itoa(entradasSegundoNivel), data);

            entradasSegundoNivel++;
            paginasCreadas++;
        }
    entradasPrimerNivel++;

    sem_wait(&mutex_segundoNivel);
    list_add(tablasDeSegundoNivel, tablaSegundoNivel);
    sem_post(&mutex_segundoNivel);
    
    valueTablaPrimerNivel* tabla_segundo = malloc(sizeof(valueTablaPrimerNivel));
    tabla_segundo -> nroTablaNivelDos = list_size(tablasDeSegundoNivel)-1;

    //log_warning(logger,"GUARDÉ TABLA SEGUNDO NIVEL %d EN LA ENTRADA %s", tabla_segundo -> nroTablaNivelDos, string_itoa(entradasPrimerNivel-1) );

    dictionary_put(tablaPrimerNivel, string_itoa(entradasPrimerNivel-1), tabla_segundo);   // ojo con el cast // no more cast
    }

    sem_wait(&mutex_primerNivel);
    list_add(tablasDePrimerNivel, tablaPrimerNivel);
    sem_post(&mutex_primerNivel);

    return list_size(tablasDePrimerNivel) - 1; 
}


void inicializarProceso(uint32_t pid, uint32_t tamProceso) {
    //while(1) {
        //sem_wait(&sem_inicializarProceso); // poner el signal en server_escuchar (o algo por ahí)

        crearArchivo(pid, tamProceso);
        uint32_t tablaProcesoPrimerNivel = generarEstructuras(pid, tamProceso);

        punteroID* punteroNuevo = malloc(sizeof(punteroID));

        punteroNuevo ->puntero = 0;

        dictionary_put(punterosPorProceso,string_itoa(pid), punteroNuevo);

        send_dos_uint(fd_kernel_cpu, pid, tablaProcesoPrimerNivel);
        
    //}
}
////////////////////////////////////////////////////////////////////    SUSPENDER PROCESO  //////////////////////////////////////////////////////////////////////////////////////////////////////


void copiarEnDisco(uint32_t pid, uint32_t marco, uint32_t nroPagina) {
    
    char* path = generarPath(pid);
    void* contenidoPagina = malloc(config -> tam_pagina);
    memcpy(contenidoPagina, espacioUsuario + marco * (config -> tam_pagina), config -> tam_pagina);
    
    sem_wait(&mutex_SWAP);
    esperarSwap();
    sem_post(&mutex_SWAP);
    
    FILE* archivoSwap = fopen(path, "rb");
    fseek(archivoSwap, 0, SEEK_SET);
    uint32_t tamanio = tamanioActualDelArchivo(archivoSwap);
    
    void* contenidoArchivo = malloc(tamanio);
    fread(contenidoArchivo, tamanio, 1, archivoSwap);
  
    memcpy(contenidoArchivo + nroPagina * (config -> tam_pagina), contenidoPagina, (config -> tam_pagina));

    fclose(archivoSwap);
    archivoSwap = fopen(path, "wb");
    fseek(archivoSwap, 0, SEEK_SET);

    fwrite(contenidoArchivo, tamanio, 1, archivoSwap);
 
    fclose(archivoSwap);

    free(contenidoArchivo);
 
    free(contenidoPagina);
    
    sem_post(&swapListo);
} 

void sacarPaginaDeMemoria(datosPagNumerada* paginaAEliminar) {
    
    uint32_t marcoPagEliminar = paginaAEliminar -> dataPagina -> marco;

    if(paginaAEliminar -> dataPagina -> modificado){
            
        copiarEnDisco(tablasCPU -> pidProceso,(paginaAEliminar -> dataPagina) -> marco, paginaAEliminar -> nroPagina);
        
    }
          
    paginaAEliminar -> dataPagina-> modificado = false; 
    paginaAEliminar -> dataPagina-> presencia  = false;
    paginaAEliminar -> dataPagina-> uso        = false;
    paginaAEliminar -> dataPagina-> marco      =    -1;

    valueTablaMarcos* value = dictionary_get(tablaMarcos, string_itoa(marcoPagEliminar));

    value -> libre = true;
 
} 

void liberarMarcos(uint32_t pidProceso) {
    for(int i =0; i < dictionary_size(tablaMarcos); i++){
        valueTablaMarcos* value = dictionary_get(tablaMarcos,string_itoa(i));
        
        if((value -> pid) == pidProceso) {
            value -> pid   = -1;
            value -> libre = true;
        }
    }
}

void desSuspenderProceso(uint32_t pid) {
    sem_wait(&mutex_tablaMarcos);
    asignarMarcos(pid);
    sem_post(&mutex_tablaMarcos);

    //log_info(logger, "Le reservé marcos al proceso %d", pid);
}

void suspenderProceso(uint32_t pid, uint32_t tabla_paginas) {

    //while(1) {
        //sem_wait(&sem_suspenderProceso);
        sem_wait(&mutex_primerNivel);
        sem_wait(&mutex_segundoNivel);

        t_dictionary* tablaPrimerNivel = list_get(tablasDePrimerNivel,tabla_paginas);

        punteroID* punteroObtenido = dictionary_get(punterosPorProceso,string_itoa(pid));
        punteroObtenido ->puntero = 0;
        
        uint32_t nroPag = 0;

        

        for(int i = 0; i < (dictionary_size(tablaPrimerNivel));i++){
            valueTablaPrimerNivel* tabla_segundo = dictionary_get(tablaPrimerNivel, string_itoa(i));
           
            uint32_t nroTablaSegundoNivel = tabla_segundo -> nroTablaNivelDos;
            
            t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, nroTablaSegundoNivel);
            

            for (int j = 0;j < (dictionary_size(tablaSegundoNivel)); j++){
                
                datosPagina* data = dictionary_get(tablaSegundoNivel, string_itoa(j));
               
                if(data->presencia){
                    
                    datosPagNumerada* paginaEnMemoria = malloc(sizeof(paginaEnMemoria));
                    paginaEnMemoria -> dataPagina = data;
                    paginaEnMemoria -> nroPagina = nroPag;
                    
                    sacarPaginaDeMemoria(paginaEnMemoria);  
                    free(paginaEnMemoria);  // SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO SI ROMPE BORRAR ESTO
                }
               
                nroPag++;
            }
        }
        
        sem_post(&mutex_segundoNivel);
        sem_post(&mutex_primerNivel);
        
        sem_wait(&mutex_tablaMarcos);
        liberarMarcos(pid);
        sem_post(&mutex_tablaMarcos);

    //}
}
////////////////////////////////////////////////////////////////////    FINALIZAR PROCESO  //////////////////////////////////////////////////////////////////////////////////////////////////////


void eliminarProcesoDelDisco(uint32_t pid) {
    char* path = generarPath(pid);
    remove(path);
}

void finalizarProceso(uint32_t pid, uint32_t tabla_paginas) {

    //while(1) {
        //sem_wait(&sem_finalizarProceso);
        sem_wait(&mutex_primerNivel);
        sem_wait(&mutex_segundoNivel);

        t_dictionary* tablaPrimerNivel = list_get(tablasDePrimerNivel,tabla_paginas);
        
        uint32_t nroPagina = 0;

        for(int i = 0; i < (dictionary_size(tablaPrimerNivel));i++){
            
            valueTablaPrimerNivel* tabla_segundo = dictionary_get(tablaPrimerNivel, string_itoa(i));
            uint32_t nroTablaSegundoNivel = tabla_segundo -> nroTablaNivelDos;
            t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, nroTablaSegundoNivel);

            for (int j = 0;j < (dictionary_size(tablaSegundoNivel)); j++){
                
                datosPagina* data = dictionary_get(tablaSegundoNivel, string_itoa(j));

                if(data->presencia){
                    data->presencia = false;
                    data->marco = -1;
                }
                sem_wait(&mutex_tablaMarcos);
                liberarMarcos(pid);
                sem_post(&mutex_tablaMarcos);
                
                eliminarProcesoDelDisco(pid);
                nroPagina++;
            }
        }
        sem_post(&mutex_segundoNivel);
        sem_post(&mutex_primerNivel);
    //}
}

/////////////////////////////////////////////////////////RESPUESTAS A CPU//////////////////////////////////////////////////////////////////////

void devolverTablaSegundoNivel(uint32_t nroTabla, uint32_t entrada) {
    tablasCPU -> nroTablaPrimerNivel = nroTabla;
    
    sem_wait(&mutex_primerNivel);
    
    t_dictionary* tablaPrimerNivel = list_get(tablasDePrimerNivel, tablasCPU -> nroTablaPrimerNivel);
    
    sem_post(&mutex_primerNivel);
    valueTablaPrimerNivel* tabla_segundo = dictionary_get(tablaPrimerNivel, string_itoa(entrada));
    
    uint32_t nroTablaSegundoNivel = tabla_segundo -> nroTablaNivelDos;
    
    tablasCPU -> nroTablaSegundoNivel = nroTablaSegundoNivel;
    
    log_info(logger,"\n\nENVIO A CPU LA TABLA DE SEGUNDO NIVEL, NÚMERO: %d Y CORRESPONDE A LA ENTRADA: %d\n\n", nroTablaSegundoNivel, entrada);
    
    send(fd_memoria_cpu, &nroTablaSegundoNivel, sizeof(uint32_t), 0);
}

void sacarPaginaDeDisco(uint32_t pid, uint32_t marco, uint32_t nroPagina) {
    char* path = generarPath(pid);
    void* contenidoPagina = malloc(config -> tam_pagina);
    //memcpy(contenidoPagina, espacioUsuario + marco * (config -> tam_pagina), config -> tam_pagina);

    sem_wait(&mutex_SWAP);
    esperarSwap();
    sem_post(&mutex_SWAP);

    FILE* archivoSwap = fopen(path, "rb");
    fseek(archivoSwap, 0, SEEK_SET);
    uint32_t tamanio = tamanioActualDelArchivo(archivoSwap);

    void* contenidoArchivo = malloc(tamanio);
    fread(contenidoArchivo, tamanio, 1, archivoSwap);

    memcpy(contenidoPagina, contenidoArchivo + nroPagina * (config -> tam_pagina), (config -> tam_pagina));

    fclose(archivoSwap);
    free(contenidoArchivo);

    memcpy(espacioUsuario + marco * (config -> tam_pagina), contenidoPagina, (config -> tam_pagina));
    free(contenidoPagina);
    sem_post(&swapListo);
}

void cargarPaginaEnMemoria(datosPagNumerada* paginaACargar, uint32_t marcoDestino) {
    paginaACargar -> dataPagina -> uso          = true;
    paginaACargar -> dataPagina -> presencia    = true;
    paginaACargar -> dataPagina -> modificado   = false;
    paginaACargar -> dataPagina -> marco        = marcoDestino;
    sacarPaginaDeDisco(tablasCPU -> pidProceso, marcoDestino, paginaACargar -> nroPagina);

    //log_error(logger, "LA PÁGINA %d AHORA ESTÁ EN EL MARCO %d", paginaACargar -> nroPagina, marcoDestino);

    sem_wait(&mutex_tablaMarcos);
    valueTablaMarcos* value = dictionary_get(tablaMarcos, string_itoa(marcoDestino));
    value -> libre = false;
    sem_post(&mutex_tablaMarcos);
}

void* algoritmo_clock(t_list* listaDatosPagina,uint32_t pid) {
	int flag_reemplazo = 0;
    datosPagNumerada* dato_a_reemplazar;
    int i;

    punteroID* punteroObtenido = dictionary_get(punterosPorProceso,string_itoa(pid));
	
	for(i=(punteroObtenido ->puntero);i<list_size(listaDatosPagina);i++){
		dato_a_reemplazar = list_get(listaDatosPagina,i);

        //printf("encontre el marco es %d \n",dato_a_reemplazar ->dataPagina ->marco);
       // printf("el valor de i es %d \n",i);
        //printf("el valor de puntero es %d \n",punteroObtenido -> puntero);

		if(dato_a_reemplazar -> dataPagina -> uso == 1){
			dato_a_reemplazar -> dataPagina -> uso = 0;	
		}
		else if(dato_a_reemplazar -> dataPagina -> uso == 0){
			flag_reemplazo = 1;
			punteroObtenido ->puntero = i;
			break;
		}
	}

	if(flag_reemplazo == 0)
    {
        punteroObtenido ->puntero = 0;
        
        return algoritmo_clock(listaDatosPagina,pid);
    }
	else {

        if(punteroObtenido -> puntero ==(list_size(listaDatosPagina)-1)){
           punteroObtenido ->puntero = 0;
        }
        else{
            punteroObtenido ->puntero = punteroObtenido ->puntero + 1;
        }
        //log_warning(logger, "encontre el marco es %d y el valor de puntero es %d",dato_a_reemplazar ->dataPagina ->marco, punteroObtenido ->puntero);
		return dato_a_reemplazar;
    }
}




///////////////////////////////////////////////////////
///////////////////////////////////////////////////////





void* algoritmo_clock_mejorado(t_list* listaDatosPagina,uint32_t pid) {
	int flag_reemplazo = 0;
    datosPagNumerada* dato_a_reemplazar;
	int i;
    int x;

    punteroID* punteroObtenido = dictionary_get(punterosPorProceso,string_itoa(pid));

	if(nro_vuelta == 0){
		for(i=(punteroObtenido ->puntero);i<list_size(listaDatosPagina);i++){

			dato_a_reemplazar = list_get(listaDatosPagina,i);

            //printf("encontre el marco es %d \n",dato_a_reemplazar ->dataPagina ->marco);
            //printf("el valor de i es %d \n",i);
            //printf("el valor de puntero es %d \n",punteroObtenido ->puntero);

            //log_error(logger, "el valor del bit de uso es %d",dato_a_reemplazar -> dataPagina -> uso);
            //log_error(logger, "el valor del bit de modificado es %d",dato_a_reemplazar -> dataPagina -> modificado);
		
			if(dato_a_reemplazar -> dataPagina -> uso == 0 && dato_a_reemplazar -> dataPagina -> modificado == 0){
				flag_reemplazo = 1;	
				punteroObtenido ->puntero = i;

				break;
			}
		}	
	}
	else if(nro_vuelta == 1 && flag_reemplazo == 0){
		//puntero = 0;
		for(x=(punteroObtenido -> puntero);x<list_size(listaDatosPagina);x++){

			dato_a_reemplazar = list_get(listaDatosPagina,x);

            //printf("encontre el marco es %d \n",dato_a_reemplazar ->dataPagina ->marco);
            //printf("el valor de x es %d \n",x);
            //printf("el valor de puntero es %d \n",punteroObtenido ->puntero);
		
			if(dato_a_reemplazar -> dataPagina -> uso == 0 && dato_a_reemplazar -> dataPagina -> modificado == 1){
				flag_reemplazo = 1;
				punteroObtenido ->puntero = x;
				break;
			}
			else{
				dato_a_reemplazar -> dataPagina -> uso = 0;
			}
		}	
	}
	
	if(flag_reemplazo==0){
		if(nro_vuelta== 0)
			nro_vuelta = 1;
		else
			nro_vuelta = 0;
		
		punteroObtenido ->puntero = 0;
        //log_warning(logger, "hice una vuelta");
		return algoritmo_clock_mejorado(listaDatosPagina,pid);
	}
	else {
        
        nro_vuelta = 0;

        if(punteroObtenido -> puntero ==(list_size(listaDatosPagina)-1)){
            punteroObtenido ->puntero = 0;
        }
        else{
            punteroObtenido ->puntero = punteroObtenido ->puntero + 1;
        }

        //log_warning(logger, "encontre el marco es %d y el valor de puntero es %d",dato_a_reemplazar ->dataPagina ->marco, punteroObtenido ->puntero);

		return dato_a_reemplazar;
    }
}


uint32_t aplicarAlgoritmoReemplazo(t_list* paginasEnMemoria, datosPagNumerada* paginaACargar,uint32_t pid) {
	
	datosPagNumerada* paginaAEliminar;
	
	if(strcmp(config -> algoritmo_reemplazo,"CLOCK") == 0){	
        
		paginaAEliminar = algoritmo_clock(paginasEnMemoria,pid); // QUE DEVUELVA UNA PAGINA NUMERADA
        log_info(logger, "Aplique algortimo CLOCK, para el proceso %d, y elimine la pagina %d del marco %d", pid, paginaAEliminar -> nroPagina,  paginaAEliminar -> dataPagina -> marco);
	}
	
	else if(strcmp(config -> algoritmo_reemplazo,"CLOCK-M") == 0){
		paginaAEliminar = algoritmo_clock_mejorado(paginasEnMemoria,pid); // QUE DEVUELVA UNA PAGINA NUMERADA
        log_info(logger, "Aplique algortimo CLOCK-M, para el proceso %d, y elimine la pagina %d del marco %d", pid, paginaAEliminar -> nroPagina,  paginaAEliminar -> dataPagina -> marco);
	}

    uint32_t marco = paginaAEliminar -> dataPagina -> marco;

    sacarPaginaDeMemoria(paginaAEliminar);
    cargarPaginaEnMemoria(paginaACargar, marco);

    return marco;
}


uint32_t buscarMarcoLibre(uint32_t pidBuscado) {
    valueTablaMarcos* value;
    uint32_t marco;
    for (marco = 0; marco < dictionary_size(tablaMarcos); marco++) {
        value = dictionary_get(tablaMarcos, string_itoa(marco));
        if (value -> pid == pidBuscado) {
            for (uint32_t i = 0; i < config -> marcos_por_proceso; i++) {
                value = dictionary_get(tablaMarcos, string_itoa(marco + i));
                if (value -> libre) {
                    log_info(logger, "Encontré el marco %d para el proceso %d.", (marco + i), pidBuscado);
                    return (marco + i);
                }
            }
            log_warning(logger, "El proceso %d no tiene marcos libres.", pidBuscado);
            return -1;
        }
    }
    log_error(logger, "El proceso %d no tiene marcos reservados.", pidBuscado);
    return -2;
}

static bool marco_menor(datosPagNumerada* paginaEnMemoria1,datosPagNumerada* paginaEnMemoria2) {
    return paginaEnMemoria1 -> dataPagina -> marco <= paginaEnMemoria2 -> dataPagina -> marco;
}

uint32_t reemplazarPagina(uint32_t pid, datosPagNumerada* paginaACargar) {
    t_list* paginasEnMemoria;
    paginasEnMemoria = list_create();
    uint32_t nroPag = 0;

    sem_wait(&mutex_primerNivel);
    t_dictionary* tablaPrimerNivel = list_get(tablasDePrimerNivel,tablasCPU -> nroTablaPrimerNivel);
    sem_post(&mutex_primerNivel);

    for(int i = 0; i < dictionary_size(tablaPrimerNivel); i++){

        valueTablaPrimerNivel* tabla_segundo = dictionary_get(tablaPrimerNivel, string_itoa(i));
        uint32_t nroTablaSegundoNivel = tabla_segundo -> nroTablaNivelDos;
        
        sem_wait(&mutex_segundoNivel);
        t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, nroTablaSegundoNivel);
        sem_post(&mutex_segundoNivel);

        for (int j = 0; j < dictionary_size(tablaSegundoNivel); j++){
            datosPagina* pagina = dictionary_get(tablaSegundoNivel, string_itoa(j));
            if(pagina -> presencia ){
                datosPagNumerada* paginaEnMemoria = malloc(sizeof(paginaEnMemoria));
                paginaEnMemoria -> dataPagina = pagina;
                paginaEnMemoria -> nroPagina = nroPag;

                list_add(paginasEnMemoria,paginaEnMemoria);
            }
            nroPag++;
        }
    }
        
        //log_error(logger, "\nEL tamanio de la lista ANTES de sort es: %d",list_size(paginasEnMemoria));

        list_sort(paginasEnMemoria, (void*) marco_menor);

        //log_error(logger, "\nEL tamanio de la lista DESPUES de sort es: %d",list_size(paginasEnMemoria));

        uint32_t marcoReemplazado = aplicarAlgoritmoReemplazo(paginasEnMemoria, paginaACargar,pid); //aca adentro se sacaUna pag de memoria y se agrega la otra
        list_destroy_and_destroy_elements(paginasEnMemoria,free);
        return marcoReemplazado;
}

void agregarPagina(uint32_t pid, datosPagNumerada* paginaACargar) {
    
    sem_wait(&mutex_tablaMarcos);
    //log_warning(logger, "llegue a antes de buscarMARCOLIBRE");
    uint32_t marcoLibre = buscarMarcoLibre(pid);
    //log_info(logger, "El marco a asignar es: %d",marcoLibre);
    
    sem_post(&mutex_tablaMarcos);

    switch (marcoLibre) {
        case -2:
            // caso en el que no tiene marcos asignados, error fuerte.
            break;
        case -1:
            marcoLibre = reemplazarPagina(pid, paginaACargar);
            break;
        default:
            cargarPaginaEnMemoria(paginaACargar, marcoLibre);
            break;
    }

    send(fd_memoria_cpu, &marcoLibre, sizeof(uint32_t), 0);
}

uint32_t obtenerNumeroDePagina(uint32_t entradaPrimerNivel, uint32_t entradaSegundoNivel) {
    return ( ( entradaPrimerNivel * (config -> entradas_por_tabla) ) + entradaSegundoNivel );
}

void devolverMarco(uint32_t pid, uint32_t nroTablaSegundoNivel, uint32_t entradaTablaSegundoNivel) { 

    sem_wait(&mutex_segundoNivel);
    t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, nroTablaSegundoNivel);
    sem_post(&mutex_segundoNivel);
 

    datosPagina* dataPag = dictionary_get(tablaSegundoNivel, string_itoa(entradaTablaSegundoNivel));
    uint32_t numPagina = obtenerNumeroDePagina(tablasCPU -> entradaPrimerNivel, entradaTablaSegundoNivel);

    datosPagNumerada* dataNumerada = malloc(sizeof(dataNumerada));
    dataNumerada -> dataPagina = dataPag;
    dataNumerada -> nroPagina = numPagina;

    

    if(!(dataPag->presencia)){
        
        agregarPagina(pid, dataNumerada);
    }
    else
    {
        //log_warning(logger, "Entré al else de presencia, la pagina estaba con bit p = 1");
        send(fd_memoria_cpu, &(dataNumerada -> dataPagina -> marco), sizeof(uint32_t), 0);
    }
    log_info(logger, "Mandé a cpu el marco: %d", dataNumerada -> dataPagina -> marco);
}

void* marcoCorrespondienteWrite(uint32_t tabla_paginas, uint32_t df) {
	
	// busco el número de marco
	uint32_t nroDeMarco = floorCasero( df/(config->tam_pagina) );
	
	// busco la página que esté en ese marco y la retorno
	
	t_dictionary* tablaPrimerNivel = list_get(tablasDePrimerNivel, tabla_paginas);
	
	for (int i = 0; i < (dictionary_size(tablaPrimerNivel)); i++) {
		
		valueTablaPrimerNivel* tabla_segundo = dictionary_get(tablaPrimerNivel, string_itoa(i));
        uint32_t nroTablaSegundoNivel = tabla_segundo -> nroTablaNivelDos;
		t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, nroTablaSegundoNivel);
		
		for (int j = 0; j < (dictionary_size(tablaSegundoNivel)) ; j++) {
			
			datosPagina* data = dictionary_get(tablaSegundoNivel, string_itoa(j));
			
			if (data -> marco == nroDeMarco) {
				//log_info(logger,"La página cuyo bit de modificado tengo que poner en 1 es la del marco %d", data -> marco);
				return data; // también podemos directamente modificar acá el bit de modificado y no preocuparnos por los punteros
			}
		}
	}
	
	return NULL; // o algún código de error (estaba -1 antes)
}


void leer(uint32_t df) {//SOLO READ O WRITE (COPY SON DOS MENSAJES DISTINTOS)
    uint32_t valorLeido;
    memcpy(&valorLeido, espacioUsuario + df, sizeof(uint32_t));

    log_info(logger, "Voy a mandar a cpu el valor: %d ",valorLeido);

    send(fd_memoria_cpu, &valorLeido, sizeof(uint32_t), 0);
}



void escribir(uint32_t df, uint32_t valor) {
    memcpy(espacioUsuario + df, &valor, sizeof(uint32_t));
    
    //t_dictionary* tablaSegundoNivel = list_get(tablasDeSegundoNivel, tablasCPU -> nroTablaSegundoNivel);
    //datosPagina* data = dictionary_get(tablaSegundoNivel, string_itoa(tablasCPU -> entradaSegundoNivel));

    datosPagina* data = marcoCorrespondienteWrite((tablasCPU -> nroTablaPrimerNivel), df);
    data -> modificado = true;

    char* ok = "OK";
    log_info(logger, "Escribi lo solicitado por cpu: %d ", valor);
    send_mensaje_consola(fd_memoria_cpu, ok); // es send_mensaje_consola pero no se lo mandamos a consola
}


////////////////////////////////////// ESCUCHAS //////////////////////////////////////////////////////

void* escucharCPU() {
	char* mensajeRecibido;
	uint32_t pidRecibido;
	uint32_t primerInt;
	uint32_t segundoInt;
	log_info(logger, "Esperando handshake con CPU.");
	//sem_wait(&sem_handshakeCPU);
	while(recv_char_y_tres_int(fd_memoria_cpu, &mensajeRecibido, &pidRecibido, &primerInt, &segundoInt)) { // JUAMPI <3
		//log_info(logger, "recibi el mensaje de cpu");
        //log_error(logger,"RECIBI EL MENSAJE POR CPU, con los valores %s ,%d ,%d ,%d",mensajeRecibido,pidRecibido,primerInt,segundoInt);
        
        tablasCPU -> pidProceso = pidRecibido;

        usleep((config -> retardo_memoria) * 1000);
        
		if (strcmp(mensajeRecibido, "QUIERO_SEGUNDO_NIVEL") == 0) {
            tablasCPU -> entradaPrimerNivel = segundoInt;            
			devolverTablaSegundoNivel(primerInt, segundoInt);	
		} else 
		if (strcmp(mensajeRecibido, "QUIERO_MARCO") == 0) {
            tablasCPU -> entradaSegundoNivel = segundoInt;
			devolverMarco(pidRecibido, primerInt, segundoInt);

		} else
		if (strcmp(mensajeRecibido, "WRITE") == 0) {
			escribir(primerInt, segundoInt);
		} else
		if (strcmp(mensajeRecibido, "READ") == 0) {
			leer(primerInt);
		} else
			log_error(logger, "No recibí ninguno de los 4 mensajes esperados, recibi %s",mensajeRecibido);		
	}
	log_error(logger, "No pude recibir un char y dos int de CPU.");
    return NULL;
}

void* escucharKernel() {
	char* mensajeRecibido;
	uint32_t pidRecibido;
	uint32_t elInt;
	log_info(logger,"Esperando conexión con el Kernel.");
	//sem_wait(&sem_handshakeKerner);
	while(recv__char_y_dos_int(fd_kernel_cpu, &mensajeRecibido, &pidRecibido, &elInt)) { // JUAMPI <3
		if (strcmp(mensajeRecibido, "INICIALIZAR") == 0) {
			inicializarProceso(pidRecibido, elInt);
            log_warning(logger, "Cree las estructuras para el proceso %d.", pidRecibido);
		} else
		if (strcmp(mensajeRecibido, "SUSPENDER") == 0) {
            //log_error(logger,"SUSPENDER");
			suspenderProceso(pidRecibido, elInt);
            log_warning(logger, "Suspendí el proceso %d.", pidRecibido);
		} else
		if (strcmp(mensajeRecibido, "NO_SUSPENDED") == 0) {
			desSuspenderProceso(pidRecibido);
            log_warning(logger, "Des-suspendi el proceso %d.", pidRecibido);
		} else
		if (strcmp(mensajeRecibido, "FINALIZAR") == 0) {
			finalizarProceso(pidRecibido, elInt);
            log_warning(logger, "Liberé los marcos del pid %d. Y lo elimine de swap", pidRecibido);
		} else
			log_error(logger, "No recibí ninguno de los 4 mensajes esperados");		
	}
	log_error(logger, "No pude recibir un char y dos int de CPU.");
	return NULL;
}





void levantarHilos() {
    // escucharCPU
    //pthread_t hilo_escucharCPU;
    //pthread_create(&hilo_escucharCPU, NULL, escucharCPU, NULL);
    //pthread_detach(hilo_escucharCPU);

    // escucharKernel
    //pthread_t hilo_escucharKernel;
    //pthread_create(&hilo_escucharKernel, NULL, escucharKernel, NULL);
    //pthread_detach(hilo_escucharKernel);

    // SWAP
    //pthread_t hilo_SWAP;
    //log_error(logger, "FALTA EL HILO DE SWAP");
    //log_warning(logger, "levantarHilos() no hace nada...");
}

void inicializarMemoria() {
    config_get_string_values();
    init_listas();
    init_semaforos();
    init_espacioUsuario();
    init_tablaMarcos();
    levantarHilos();
}
