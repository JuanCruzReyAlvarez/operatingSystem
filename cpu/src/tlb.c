#include "tlb.h" 

char* readReemplazoTLBFromConfig(){
		t_config* nuevo_config;
		char* algoritmo;
		nuevo_config = config_create("cfg/cpu.config"); 
		algoritmo = config_get_string_value(nuevo_config,"REEMPLAZO_TLB"); 
        log_info(logger_tlb, "ALGORITMO A UTILIZAR: %s ", algoritmo );      
		config_destroy(nuevo_config);
		return algoritmo;
}

int readEntradasTLBFromConfig(){
        t_config* nuevo_config;
        int limiteTabla;
        nuevo_config = config_create("cfg/cpu.config"); 
        limiteTabla = atof(config_get_string_value(nuevo_config,"ENTRADAS_TLB")); // al final el atof era al pedo se podia usar config_get_int_value y hay varias 
        config_destroy(nuevo_config); 
        log_info(logger_tlb, "LIMITE DE TABLA TLB: %d ", limiteTabla );                                                                                                                     //mas.
        return limiteTabla;
}

void config_get_string_values(t_config* nuevo_config){

    datosConfig* configNuevo = malloc(sizeof(datosConfig));

    nuevo_config = config_create("cfg/cpu.config"); // "cfg/kernel.config"

    char* reemplazoTLB = config_get_string_value(nuevo_config,"REEMPLAZO_TLB");
    uint32_t entradasTLB = (uint32_t) atoi(config_get_string_value(nuevo_config,"ENTRADAS_TLB"));
    
    configNuevo -> algoritmo_reemplazo = reemplazoTLB;
    configNuevo -> cant_entradas_tlb = entradasTLB;


    //config_destroy(nuevo_config);

    config = configNuevo;    
}



int tlbWasUsed(){
    return dictionary_size(rowTLB);   
}
void* assessDireccionLogica(uint32_t* marco, char* pagina_key){

        //printf("\n\n ENTRE A ACCESS DIRECCION LOGICA \n\n");

        //char* algortimoDeReemplazoDePagina = readReemplazoTLBFromConfig();

        //printf("\n\nEl algoritmo leído es %s.\n\n",config -> algoritmo_reemplazo);  // ACÁ ESTÁ FALLANDO!!!!! lee mal la config

        algoritmoTLB algoritmo = generarEnumAlgoritmo(config -> algoritmo_reemplazo); // y por ende acá rompe


        switch(algoritmo){
            case FIFO:
		{
			replaceByFifo(marco,pagina_key);
			break;
		}
        case LRU:
		{
			replaceByLru(marco,pagina_key);
			break;
		}
    }
    return NULL;
}

void* executeRowTLB(pcb* pcb, uint32_t direccion_logica,uint32_t* marco,uint32_t cant_entradas_por_tabla,
uint32_t tablaPagina_1er_nivel, char* pagina_key){
     // hay que agregarla de cero, o agregarla remplazandola por un algoritmo
    //int limiteTabla = readEntradasTLBFromConfig();

    //printf("El limite de tablas es: %d\n",config -> cant_entradas_tlb);

    //printf("El algoritmo de reemplazo es: %s\n",config -> algoritmo_reemplazo);

    startMemoryManagmentUnit(pcb, direccion_logica, tablaPagina_1er_nivel,cant_entradas_por_tabla, marco);

    //printf("Hice el startMemoryManagment, voy a ver la cantidad de entradas de la tlb\n");

    if (dictionary_size(rowTLB) < (config -> cant_entradas_tlb)){

        //printf("El tamaño del diccionario es: %d\n",dictionary_size(rowTLB));

       // marco = malloc(sizeof(uint32_t));
        marcoTLB* marco_TLB = malloc(sizeof(marcoTLB));
        marco_TLB -> nro_marco = *marco;

        //printf("\n\n\nGuardé la página %s y el marco %d\n\n\n", pagina_key, marco_TLB -> nro_marco);
        dictionary_put(rowTLB,  pagina_key,marco_TLB);

        //printf("PRUEBA Del marco: %d\n", marco);
        list_add(listSeguridadTLB,pagina_key);
        list_add(timeStateTablaDirecciones,pagina_key);
        log_info(logger_tlb, "TLB TIENE ESPACIOS LIBRES. NO SE EJECUTAN ALGORITMOS DE REEMPLAZO. SE AGREGAN PAGINA Y MARCO A LA TLB.");
    }else{
            //printf("Entro al ELSE\n");
            assessDireccionLogica(marco, pagina_key);
            log_info(logger_tlb, "TLB NO TIENE ESPACIOS LIBRES. SE EJECUTAN ALGORITMOS DE REEMPLAZO.");
    }
    return NULL;
}

void* runTranslationLookasideBuffer(pcb* pcb,uint32_t direccion_logica,uint32_t tablaPagina_1er_nivel,uint32_t cant_entradas_por_tabla,uint32_t* marco)
{
    
    char* pagina_key = string_itoa(direccion_logica/tamanio_de_la_pagina); // chekear esa funcion.

    //printf("El valor de pagina_key en runTLB es: %s , y lo calculo con los valores %d, %d\n",pagina_key,direccion_logica,tamanio_de_la_pagina);
/*
    for(int i=0; i<list_size(listSeguridadTLB); i++) {
        char* pag = list_get (listSeguridadTLB, i);
        printf("\n\nENTRADA %d DE LA TLB = %s\n\n", i, pag);
    }
*/
    if(dictionary_has_key(rowTLB,pagina_key)){

        
        marcoTLB* marco_TLB; // = malloc(sizeof(marcoTLB)); 
        marco_TLB = dictionary_get(rowTLB, pagina_key);  
        *marco = marco_TLB -> nro_marco;
        //printf("EL MARCO ANTES DE GUARDARLO ES %d\n", marco_TLB -> nro_marco);

        printf("TENGO LA PAGINA NRO %s EN LA TLB!! \n",pagina_key);
        
        printf("Y EL MARCO TIENE VALOR con *: %d \n",*marco);

        if (strcmp(config -> algoritmo_reemplazo, "LRU") == 0) {
            //list_remove(timeStateTablaDirecciones, la página);
            list_add(timeStateTablaDirecciones, pagina_key);
        }
        log_info(logger_tlb, "TLB TIENE EL MARCO DE LA PAGINA, NO NECESITO ACCEDER A MEMORA EN BUSCA DE EL.");
    }
    else{
        //printf("NO tengo el marco en la tlb, hago executeRowTLB\n");
        log_info(logger_tlb, "TLB NO TIENE EL MARCO DE LA PAGINA, NECESITA ACCEDER A MEMORA EN BUSCA DEL MISMO.");
        executeRowTLB(pcb, direccion_logica, marco,cant_entradas_por_tabla, tablaPagina_1er_nivel,pagina_key);
    }

    return NULL;
}



// ojo con la direccion fisica que es int pero puede q sea unsigned int y traiga problemas de btes.

