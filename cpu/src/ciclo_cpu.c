#include "ciclo_cpu.h"  



void* actualizar_tiempo_estimado_sin_formula(pcb* pcb, time_t t_inicial, time_t t_final){
	float estimacion_anterior = pcb -> estimacion_rafaga;
	pcb -> estimacion_rafaga = ( estimacion_anterior - difftime(t_final, t_inicial) * 1000 );
	return NULL;
}



char* puertoDispatch(){
    t_config* nuevo_config;
	nuevo_config = config_create("cfg/cpu.config"); // "cfg/kernel.config"
    return config_get_string_value(nuevo_config,"PUERTO_ESCUCHA_DISPATCH");
}



t_log* crear_logger_cpu(){
	return log_create("./cfg/cpu.log", "CPU", true, LOG_LEVEL_INFO);
}


void cpu_log_handshake(t_log* logger){                                                      
    log_info(logger, "Conexion a memoria y handshake realizado con exito");
}  


float read_retardo_op_from_config(){
        t_config* nuevo_config;
        float op;
		nuevo_config = config_create("cfg/cpu.config"); 
        op = atof(config_get_string_value(nuevo_config,"RETARDO_NOOP")); 
        config_destroy(nuevo_config);
        return op;
}

float calcular_estimacion_rafaga(float cpu_usado, float alfa, float est_ant){

    return ( alfa * cpu_usado ) + ( 1 - alfa ) * est_ant ;

}
void* actualizar_tiempo_estimado(pcb* pcb, time_t t_inicial, time_t t_final){
	float cpu_usado = difftime(t_final, t_inicial) * 1000 ;   // Se multiplica por 1000 para que pase de segundos a milisegundos.
	float estimacion_vieja = pcb -> estimacion_rafaga ;  
	//float alfa = read_alfa_from_config();
	//log_warning(logger,"El valor de alfa en actualizar es: %f",alfa_recibido);
	float nueva_estimacion = calcular_estimacion_rafaga(cpu_usado, alfa_recibido, estimacion_vieja);
	
	//log_error(logger, "\n\nt_incial: %ld\nt_final: %ld\nCPU USADO: %f\nESTIMACION VIEJA: %f\nESTIMACION NUEVA:%f", t_inicial, t_final, cpu_usado, estimacion_vieja, nueva_estimacion);
	pcb -> estimacion_rafaga = nueva_estimacion;
	return NULL;
}

/*void cleanCaseroTLB(t_dictionary* tlb) {
	while (!dictionary_is_empty(tlb)) {
		log_warning(logger, "Entré al dictionary is empty");
		marcoTLB* row = dictionary_remove(tlb, string_itoa(0));
		log_warning(logger, "Voy a eliminar el marco %d", row -> nro_marco);
		free(row);
	}
}*/

void resetTLB(){

	//t_list* reset_lists = list_create();
	//t_dictionary* reset_dictionary = dictionary_create();
    
	dictionary_clean_and_destroy_elements(rowTLB, free );
	//cleanCaseroTLB(rowTLB);
	list_clean(timeStateTablaDirecciones);

	
	
}

void start_again(pcb* pcb){

	//pcb* pcbNueva = malloc(sizeof(pcb)); //ver si hacer malloc o no

		

		if(tlbWasUsed()>=1){
			resetTLB();
			log_warning(logger, "TLB RESETEADA.");
		}
		
		log_info(logger, "CICLO DE INSTRUCCIONES TERMINADO PARA LA PCB.");
		log_info(logger, "LISTO PARA RECIBIR NUEVA PCB");
		if(!recv_PCB(fd_dispatch,pcb)){ //VA A SER BLOQUEANTE
				log_error(logger,"HUBO UN PROBLEMA Y NO SE PUDO RECIBIR LA PCB "); // ENTRA ACA SI HAY ERRORES DE ENVIO
			}
		else{
				t_inicial = time(NULL);
				iniciar_CPU(pcb,logger);
			}

}

void* runReadConnection(pcb* pcb, uint32_t direccionFisica, uint32_t* valor){
			send_char_y_tres_int(fd_cpu_a_memoria, "READ", pcb -> id, direccionFisica, 0);
			recv(fd_cpu_a_memoria, valor, sizeof(uint32_t), MSG_WAITALL);
			return NULL;
	}

void* runWriteConnection(pcb* pcb,uint32_t  direccionFisica, uint32_t valor){
		char* confirmacion;
		send_char_y_tres_int(fd_cpu_a_memoria, "WRITE", pcb -> id, direccionFisica, valor);

		if(!recv_mensaje_consola(fd_cpu_a_memoria,&confirmacion))
			log_error(logger,"NO SE PUDO RECIBIR UN MENSAJE DE OK, FALLO , algo anda mal!");
		else
			log_warning(logger,"SE RECIBIO EL MENSAJE: %s",confirmacion);
		return NULL;
	}

//void printValor(uint32_t valor){
		//printf("LECTURA VALOR OBTENIDO:%d\n",valor); 
	//}

void* refreshTime(pcb* pcb,time_t t_inicial){
	time_t t_final = time( NULL );
	actualizar_tiempo_estimado(pcb,t_inicial,t_final);
	return NULL;
}

void* ejecutarNO_OP(pcb* pcb,time_t t_inicial){
	float retardo_op = read_retardo_op_from_config();
	//printf("llegue a leer el retardo\n");
	sleep(retardo_op / 1000);                         
	//refreshTime(pcb,t_inicial);
	//log_error(logger,"LA PCB QUE ESTA HACIENDO EL NO_OP ES: %d",pcb ->id);
	log_info(logger, "NO_OP EJECUTADO");
	return NULL;
}

void* ejecutarIO(pcb* pcb, uint32_t parametros[2], time_t t_inicial){
	
	uint32_t  tiempo_de_bloqueo =  parametros[0] ; // PARA ENVIAR A KERNEL      // 
	
	pcb -> estado_proceso = BLOCKED;

	refreshTime(pcb,t_inicial);
	//log_error(logger,"La estimacion es %d",pcb -> estimacion_rafaga);

	
	send_PCB_mas_int ( fd_dispatch , pcb, tiempo_de_bloqueo);
	//log_warning(logger, "MANDE LA PCB EN I/O");
	log_info(logger, "I/O EJECUTADO: PCB DEVUELTA EN ESTADO BLOCKED.");
	start_again(pcb);
	return NULL;
}

void* ejecutarREAD(pcb* pcb, uint32_t parametros[2], time_t t_inicial){

	uint32_t marco;
	uint32_t direccionFisica;
	uint32_t valor;

	//uint32_t marcoFalso = 0;


	runTranslationLookasideBuffer( pcb, parametros[0], pcb->tabla_paginas , 
	cantidadEntradastablaDePagina, &marco);

	//log_warning(logger,"El valor de marco en READ es: %d", marco);

	getDireccionFisica( marco, &direccionFisica, parametros[0]);

	//log_warning(logger,"El valor de direc fisica en READ es: %d", direccionFisica);

	runReadConnection(pcb,direccionFisica,&valor);

	//printValor(valor);
	log_warning(logger, "READ EJECUTADO: VALOR LEIDO: %d \n", valor);

	//refreshTime(pcb,t_inicial);

	return NULL;
}




void* ejecutarWRITE(pcb* pcb, uint32_t parametros[2], time_t t_inicial){      //si tiramos basura a lo pavote OJO CON EL PARAMETRO[2].

	uint32_t marco;
	uint32_t  direccionFisica;
	//log_warning(logger, "entro a runTLB");
	//log_warning(logger, "parametro[0] = %d", parametros[0]);
	//log_warning(logger, "tabla = %d",pcb -> tabla_paginas);
	//log_warning(logger, "cantEntradas = %d", cantidadEntradastablaDePagina);
	//log_warning(logger, "marco = %d", marco);

	runTranslationLookasideBuffer( pcb, parametros[0], pcb->tabla_paginas ,
	cantidadEntradastablaDePagina, &marco);

	log_warning(logger,"El valor de marco a escribir es: %d", marco);

    getDireccionFisica( marco, &direccionFisica, parametros[0]);

	log_warning(logger,"El valor de direccion fisica a escribir es: %d", direccionFisica);

	runWriteConnection(pcb,direccionFisica, parametros[1]);

	log_info(logger, "WRITE EJECUTADO: VALOR ESCRITO EN DESTINO.");

	//refreshTime(pcb,t_inicial);
	
		
	return NULL;
}

void* fetch_operands(pcb* pcb, uint32_t parametros[2], uint32_t* valorOrigen){

	uint32_t marcoOrigen;
	uint32_t direccionFisicaOrigen;


	runTranslationLookasideBuffer( pcb, parametros[1], pcb->tabla_paginas, 
		cantidadEntradastablaDePagina, &marcoOrigen); 

	getDireccionFisica( marcoOrigen, &direccionFisicaOrigen, parametros[1]);

	runReadConnection(pcb,direccionFisicaOrigen, valorOrigen);

	return NULL;
}

void* ejecutarCOPY(pcb* pcb, uint32_t parametros[2], time_t t_inicial, uint32_t valor ){

		uint32_t marco2;

		uint32_t direccionFisica2;

		//log_info(logger,"El valor en tabla_paginas es: %d", pcb ->tabla_paginas);


		//log_warning(logger,"El valor de marco1 es: %d", marco1);

		runTranslationLookasideBuffer( pcb, parametros[0], pcb->tabla_paginas, 
		cantidadEntradastablaDePagina, &marco2);
		
		//log_warning(logger,"El valor de marco2 es: %d", marco2);
		
		//log_warning(logger,"El valor de direc fisica 1 es: %d", direccionFisica1);

		getDireccionFisica( marco2, &direccionFisica2, parametros[0]);

		//log_warning(logger,"El valor de direc fisica 2 es: %d", direccionFisica2);

		runWriteConnection(pcb,direccionFisica2, valor);

		//refreshTime( pcb, t_inicial);
		log_info(logger, "COPY EJECUTADO: VALOR COPIADO.");
	
	return NULL;
}


void* ejecutarEXIT(pcb* pcb){
	
	pcb -> estado_proceso = FIN;
	//log_warning(logger, "TOCA MANDAR LA PCB EN FIN");
	//printf("El estado de la PCB en ejecutarEXIT es %s\n", generar_string_de_estado(pcb->estado_proceso));

/*	int a = list_size(pcb->listaInstrucciones);

        for (int i  = 0; i < a; i++){

            instruccion* inst = malloc(sizeof(inst));
            inst = list_get(pcb->listaInstrucciones, i);
            //printf("EL ID de la instruccion %d es %s\n", i, inst->id_INSTRUCCION);
            
        }
*/
	send_PCB_mas_int ( fd_dispatch , pcb, -3);
	log_info(logger, "EXIT EJECUTADO: PCB DEVUELTA EN FIN");
	start_again(pcb);   
	return NULL;                               
}

void* execute(op_code instruccion, uint32_t parametros[2], pcb* pcb, time_t t_inicial, uint32_t valor){
	//printf("llegue aca\n");
	switch(instruccion){
		
		case NO_OP:
		{
			//log_warning(logger, "NO_OP");
			//printf("llegue aca\n");
			ejecutarNO_OP(pcb,t_inicial);
			break;
		}
		case IO:
		{
			//log_warning(logger, "IO");
			ejecutarIO(pcb,parametros,t_inicial);
			//printf("llegue aca\n");
			break;
		}
		case READ:
		{
			//log_warning(logger, "READ");
			ejecutarREAD(pcb,parametros,t_inicial);
			break;
		}
		case WRITE:
		{
			//log_warning(logger, "WRITE");
			ejecutarWRITE(pcb,parametros,t_inicial);
			break;
		}
		case COPY:
		{
			//log_warning(logger, "COPY");
			ejecutarCOPY(pcb,parametros,t_inicial, valor);
			break;
		}
		case EXIT:
		{
			//printf("llegue aca medianteexit\n");
			//log_warning(logger, "EXIT");
			ejecutarEXIT(pcb);
			break;
		}
		default: log_error(logger, "NO SWITCHEO ON NIGUNA INSTRUCCION EN execute()");
	
	}      
	return NULL;	
}

void* decode(instruccion* instruccion_completa_actual,op_code* instruccion,uint32_t parametros[2]){ 
	
	*instruccion = generarCode ( instruccion_completa_actual -> id_INSTRUCCION );
	parametros[0] = instruccion_completa_actual -> param[0];
	parametros[1] = instruccion_completa_actual -> param[1];

	return NULL;
}

void* actualizar_program_counter_pcb(pcb* pcb){
	
	uint32_t cont_instruccion_anterior = pcb -> program_counter ;
	pcb -> program_counter = cont_instruccion_anterior + 1 ; 

    return NULL;
}

instruccion*  obtener_instruccion_a_ejecutar(pcb* pcb){    //malloc  //ver sacar la lista 
	
	t_list* list_instrucciones = pcb -> listaInstrucciones;
	uint32_t proxima_instruccion = pcb -> program_counter;
	return list_get(list_instrucciones,proxima_instruccion);
}

void* fetch(instruccion** instruccion,pcb* pcb){
	*instruccion = obtener_instruccion_a_ejecutar(pcb);
	actualizar_program_counter_pcb(pcb); //devuelvo struct instruccion
	return NULL;
}



void* comenzarEtapaDeEjecucion(pcb* pcb, t_log* logger ){
	
	instruccion* instruccion_completa_actual; //= malloc(sizeof(instruccion));
	op_code instruccion_op_code;
	uint32_t parametros[2];
	uint32_t valorOrigenACopiar;

	
    //printf("pcb program_counter: %d\n",pcb -> program_counter);
	//printf("pid de la pcb = %d\n",pcb -> id);
	log_info(logger,"EJECUTANDO PCB PID: %d\n",pcb -> id);

	fetch(&instruccion_completa_actual, pcb);    // Busca instruccion y modifica el program cunter pcb.
	fetch_log(logger);

    //printf("prueba instruccion argumentos primero = %d \n",instruccion_completa_actual -> param[0]);
    //printf("pcb program_counter: %d\n",pcb -> program_counter);
    

	decode(instruccion_completa_actual,&instruccion_op_code,parametros); 
	decode_log(logger);
	//log_info(logger,"FUE DECODIFICADO EL ENUM: %d \n",instruccion_op_code);

	if(instruccion_op_code == COPY){
	fetch_operands(pcb, parametros, &valorOrigenACopiar);
	fetchOperands_log(logger);
	}       


	execute(instruccion_op_code,parametros,pcb,t_inicial,valorOrigenACopiar);
	execute_log(logger);

	log_info(logger, "CICLO DE INTRUCCION TERMINADO, LISTO PARA CHEQUEAR INTERRPCIONES");
    
	//printf("Termino ciclo cpu \n");
	//free(instruccion_completa_actual);

	//printf("hice free instruccion\n");
	return NULL;
}

void* check_interrupt(pcb* pcb){
    
	char* mensaje_interrupcion_recibido;

	if(!recv_mensaje_interrupcion(fd_interrupt,&mensaje_interrupcion_recibido)){
		log_info(logger,"NO HUBO INTERRUPCIONES.");
		iniciar_CPU(pcb,logger);
	}
	else{
		log_warning(logger,"HUBO INTERRUPCIONES.SE RECIBIO EL MENSAJE %s",mensaje_interrupcion_recibido);

		//HACER UN TIMESTAMP Y CALCULAR CON LA RESTA
		time_t t_final = time( NULL );
		actualizar_tiempo_estimado_sin_formula(pcb,t_inicial,t_final);
		//log_error(logger,"La estimacion es %d",pcb -> estimacion_rafaga);

		if(!send_PCB_mas_int(fd_dispatch,pcb,-2))
			log_error(logger, "No se pudo mandar la PCB a kernel con motivo de interrupcion, hubo un fallo");
		else 
			log_info(logger, "PCB MANDADA A KERNER POR MOTIVO DE INTERRUPCION");
	
	start_again(pcb) ; 
		//en caso de que gane otra, recibe una nueva pcb recv ->

        //luego, hay que asignarle el valor a la pcb que recibimos por parametro, de la pcb nueva

		//pcb = pcbRecibida;

		//y despues arrancar devuelta iniciarCPU

		//hay que ver si recibe o no algo, ya que si recibe una pcb, va a hacer iniciar_CPU(pcbRecibida,logger);

        //sino, va a hacer iniciar_CPU(pcb,logger); como antes

	}

	return NULL;
}


void iniciar_CPU(pcb* pcb, t_log* logger )
{
		
	
    //printf("LA PRIMERA INSTRUCCION Esssss %s \n", inst -> id_INSTRUCCION);
    //printf("Y SU PARAMETRO essss %d\n", inst->param[0]);
	
    cpu_log_inicial(logger);

    //printf("Comienzo ejecucion-------------------------------------------------------------------------------\n");

	comenzarEtapaDeEjecucion(pcb,logger);
	check_interrupt(pcb); 
}