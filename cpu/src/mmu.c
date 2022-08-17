#include "mmu.h"  

void* requestTablaPagina2doNivel(pcb* pcb,uint32_t tablaPagina_1er_nivel, uint32_t entrada_tablaPagina_1er_nivel, uint32_t* tablaPagina_2do_nivel){

    //printf("EntradaTablaPagina1erNivel en REQUEST es: %d\n",tablaPagina_1er_nivel); pruebas

    //printf("el valor de fd_cpu_a_memoria en REQUEST es: %d\n", fd_cpu_a_memoria);  pruebas

    if(!send_char_y_tres_int(fd_cpu_a_memoria, "QUIERO_SEGUNDO_NIVEL", pcb -> id,tablaPagina_1er_nivel,entrada_tablaPagina_1er_nivel)){
        log_error(logger_mmu, "Error al mandar QUIERO_SEGUNDO_NIVEL");
        }
    
    recv(fd_cpu_a_memoria,tablaPagina_2do_nivel,sizeof(uint32_t),MSG_WAITALL);
    
    return NULL;
}
void* requestMarco(pcb* pcb, uint32_t tablaPagina_2do_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t* marco){
    //printf("\n\n\n\nPIDIENDO MARCO DE LA TABLA %d Y SU ENTRADA %d\n\n\n\n",tablaPagina_2do_nivel, entrada_tabla_2do_nivel );
    send_char_y_tres_int(fd_cpu_a_memoria, "QUIERO_MARCO", pcb -> id,tablaPagina_2do_nivel,entrada_tabla_2do_nivel);
    
    if(!recv(fd_cpu_a_memoria,marco,sizeof(uint32_t),MSG_WAITALL)){
        log_error(logger_mmu, "ERROR AL RECIBIR EL MARCO");
        }

    return NULL;
}
void* getNumeroPagina(uint32_t direccion_logica, uint32_t* numero_pagina){
    *numero_pagina = direccion_logica / tamanio_de_la_pagina ; 
    return NULL;
}
void* getEntradaTablaPagina1erNivel(uint32_t numero_pagina,uint32_t cant_entradas_por_tabla, uint32_t* entrada_tablaPagina_1er_nivel){
    *entrada_tablaPagina_1er_nivel = numero_pagina / cant_entradas_por_tabla;
    return NULL;
}
void* getEntradaTablaPagina2doNivel(uint32_t cant_entradas_por_tabla, uint32_t numero_pagina,uint32_t* entrada_tabla_2do_nivel){
    *entrada_tabla_2do_nivel = numero_pagina % (cant_entradas_por_tabla); // chusmear si mod es % como en c++.
    return NULL;  
}
void* getDesplazamiento(uint32_t direccion_logica,uint32_t numero_pagina, uint32_t* desplazamiento){
    *desplazamiento = direccion_logica - numero_pagina * tamanio_de_la_pagina;
    return NULL;
}

// ver como lo uno con las intrucciones 
void* getDireccionFisica(uint32_t marco, uint32_t* direccion_fisica,uint32_t direccion_logica){
    uint32_t numero_pagina;
    getNumeroPagina(direccion_logica, &numero_pagina);
    uint32_t desplazamiento;
    getDesplazamiento( direccion_logica, numero_pagina, &desplazamiento);
    *direccion_fisica = marco * tamanio_de_la_pagina +  desplazamiento;
    return NULL;
}
//---------------------

void* startMemoryManagmentUnit(pcb* pcb, uint32_t direccion_logica, uint32_t tablaPagina_1er_nivel, uint32_t cant_entradas_por_tabla,uint32_t* marco){
    
    uint32_t numero_pagina; //geteo
    uint32_t entrada_tablaPagina_1er_nivel; //geteo
    uint32_t entrada_tabla_2do_nivel; //geteo
    uint32_t desplazamiento; //geteo
    uint32_t tablaPagina_2do_nivel; //requesteo


    // marco -> driecionfisica 



    getNumeroPagina( direccion_logica, &numero_pagina );

    printf("numeroDePagina es: %d\n",numero_pagina);

    getEntradaTablaPagina1erNivel(numero_pagina, cant_entradas_por_tabla, &entrada_tablaPagina_1er_nivel);

    printf("EntradaTablaPagina1erNivel es: %d\n",entrada_tablaPagina_1er_nivel);

    getEntradaTablaPagina2doNivel(cant_entradas_por_tabla, numero_pagina, &entrada_tabla_2do_nivel);

    printf("EntradaTablaPagina2doNivel es: %d\n",entrada_tabla_2do_nivel);

    getDesplazamiento(direccion_logica, numero_pagina, &desplazamiento);

    printf("Desplazamiento es: %d\n",desplazamiento);


    requestTablaPagina2doNivel(pcb, tablaPagina_1er_nivel, entrada_tablaPagina_1er_nivel, &tablaPagina_2do_nivel);

    printf("El valor de TablaPagina2doNivel despues del REQUEST es: %d\n",tablaPagina_2do_nivel);

    requestMarco(pcb, tablaPagina_2do_nivel, entrada_tabla_2do_nivel, marco);

    printf("El valor de MARCO despues del REQUEST es: %d\n",*marco);




    return NULL;
}