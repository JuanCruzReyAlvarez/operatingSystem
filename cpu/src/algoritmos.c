#include "algoritmos.h" 

bool _is_this_pagina_LRU(char* paginaLista){
    return string_equals_ignore_case(primer_valor_lista_LRU, paginaLista);
}
bool _is_this_pagina_FIFO(char* paginaLista){
    return string_equals_ignore_case(keyToDelete, paginaLista);
}

void* deleteAndAnalyseRepeated(uint32_t marcoToDelete){
    for(int i =0 ;i < list_size(listSeguridadTLB); i++ ){
        if(!dictionary_get(rowTLB,list_get(listSeguridadTLB,i))){
            marcoTLB* marcoEliminado = dictionary_remove(rowTLB,list_get(listSeguridadTLB,i));
            free(marcoEliminado);
            i = list_size(listSeguridadTLB) + 1;
        }
    }
    return NULL;
}

void* delete_all_char(char* elementToDelete, t_list * list){
    

    int b = 0;
    t_list * lista_auxiliar = list_create();
    while( b < list_size(list) ) {

        char* to_compare =  list_get(list,b);
        
        if (strcmp(elementToDelete,to_compare) != 0)
        { 
            list_add(lista_auxiliar,to_compare);  
        }
        b++;
    }
    list_clean(list);   // and destroy elements?

    //printf("size_list_clenada%d\n",list_size(list));
    list_add_all(list,lista_auxiliar); // (listaAagregarElemenntos, lista).
    //printf("size_list_addiada%d\n",list_size(list));
    list_destroy(lista_auxiliar);
    //printf("size_list_addiadaa%d\n",list_size(list));
    
    return NULL;

}

bool  todosCharsIguales(void *elementoDeLaLista){
    return ( strcmp( elementoDeLaLista,primer_valor_lista_LRU) == 0 );
} 

void* excecLRU(){
    
    int b = list_size(timeStateTablaDirecciones); // -1

    listLRU = list_duplicate(timeStateTablaDirecciones);

    

    while(b > 0){
    

        char* pagina_a_comparar =  list_get(listLRU ,list_size(listLRU)-1);
        //printf("\n\n\n\nPÁGINA A BORRAAAAR  : %s!!!!!!!\n\n\n\n",pagina_a_comparar);



        
/*
        printf("pagina numero 1  : %s\n", (char*) list_get(listLRU,0));
        printf("pagina numero 2 : %s\n", (char*) list_get(listLRU,1));
        printf("pagina numero 3  : %s\n", (char*) list_get(listLRU,2));
        printf("pagina numero 4  : %s\n", (char*) list_get(listLRU,3));
        printf("pagina numero 5  : %s\n", (char*) list_get(listLRU,4));
        printf("pagina numero 6 : %s\n", (char*) list_get(listLRU,5));
        printf("pagina numero 7  : %s\n", (char*) list_get(listLRU,6));
        printf("pagina numero 8  : %s\n", (char*) list_get(listLRU,7));
        printf("pagina numero 9  : %s\n", (char*) list_get(listLRU,8));
        printf("pagina numero 10 : %s\n", (char*) list_get(listLRU,9));
        printf("pagina numero 11  : %s\n", (char*) list_get(listLRU,10));

        printf("------------------------------------------\n");
    */   
        
        delete_all_char(pagina_a_comparar,listLRU);
        
    /*
        printf("pagina borrada nos queda:\n");
        printf("pagina numero 1  : %s\n", (char*) list_get(listLRU,0));
        printf("pagina numero 2 : %s\n", (char*) list_get(listLRU,1));
        printf("pagina numero 3  : %s\n", (char*) list_get(listLRU,2));
        printf("pagina numero 4  : %s\n", (char*) list_get(listLRU,3)); 
        printf("pagina numero 5  : %s\n", (char*) list_get(listLRU,4));
        printf("pagina numero 6 : %s\n", (char*) list_get(listLRU,5));
        printf("pagina numero 7  : %s\n", (char*) list_get(listLRU,6));
    
        printf("------------------------------------------\n");
*/
        //printf("\n\n\n\n");
        //for(int i = 0; i<list_size(listLRU); i++) {
           //char* pag = list_get(listLRU, i);
            //printf(" - listLRU[%d] = %s\n\n", i, pag);
        //}
        if (list_size(listLRU) == 0) {
            primer_valor_lista_LRU = pagina_a_comparar;
            break;
        } else {
            primer_valor_lista_LRU = list_get(listLRU,0);
            if( list_all_satisfy(listLRU,(void*)todosCharsIguales) )
            {
                //printf("LIST ALL JOYA\n\n\n\n");
                break;
            }
            //printf("LIST ALL FALSE\n\n\n\n");
            b--;
        }
    }

    return NULL;
}

void* replaceByLru(uint32_t* marco, char* pagina){

    
    //printf("VOY A LLAMAR A EXEC\n\n\n\n");
    excecLRU(); // sabe4mos  quien sacar esta en la variable global.
    //printf("LISTO EXEC\n\n\n\n");    


    list_add(timeStateTablaDirecciones,pagina);


    //printf("VOY A LLAMAR A DELETE ALL CHAR\n\n\n\n");
    delete_all_char(primer_valor_lista_LRU,timeStateTablaDirecciones);
    //printf("DELETEE ALL CHAR\n\n\n\n");

    //printf("TOCA REMOVER LA PÁGINA %s!!!!!!!!!!!!!!!!!!!\n\n\n\n", primer_valor_lista_LRU);
    log_info(logger_tlb, "ALGORIMO LRU EJECUTADO.");
    log_info(logger_tlb, "SE ELIMINO LA PAGINA:  %s", primer_valor_lista_LRU);
    marcoTLB* marcoEliminado = dictionary_remove(rowTLB,primer_valor_lista_LRU);
    //printf("TOCA LIBERAR\n\n\n\n");
    free(marcoEliminado);
    //printf("TOCA REMOVER POR CONDICION\n\n\n\n");
    list_remove_by_condition(listSeguridadTLB, (void*) _is_this_pagina_LRU);
    
    //printf("\nLIBERÉ LA MEMORIA DEL MARCO QUE SAQUÉ DE LA TLB (LRU) :D\n");

    marcoTLB* marcoNuevo = malloc(sizeof(marcoTLB));
    marcoNuevo -> nro_marco = *marco;

    deleteAndAnalyseRepeated(marcoNuevo -> nro_marco);

    dictionary_put(rowTLB, pagina, marcoNuevo);
    log_info(logger_tlb, "SE AGREGO LA PAGINA:  %s", primer_valor_lista_LRU);
    list_add(listSeguridadTLB, pagina);

    list_clean(listLRU);   // ojo con esto si hay segfault capaz es mejor crear lista global y usarla y despues ir sobrescribiendo.

    
    
    return NULL;
}
// sacar repeticiond e logica

void* replaceByFifo(uint32_t* marco, char* pagina){
    
    keyToDelete = list_remove(timeStateTablaDirecciones,0);
	
	
    
    list_add(timeStateTablaDirecciones,pagina);

    delete_all_char(keyToDelete,timeStateTablaDirecciones);
    log_info(logger_tlb, "ALGORITMO FIFO EJECUTADO");
    log_info(logger_tlb, "SE ELIMINO LA PAGINA:  %s", keyToDelete);
    marcoTLB* marcoEliminado = dictionary_remove(rowTLB,keyToDelete);
    free(marcoEliminado);
    list_remove_by_condition(listSeguridadTLB, (void*)  _is_this_pagina_FIFO);
    //printf("\nLIBERÉ LA MEMORIA DEL MARCO QUE SAQUÉ DE LA TLB (FIFO) :D\n");

    marcoTLB* marcoNuevo = malloc(sizeof(marcoTLB));
    marcoNuevo -> nro_marco = *marco;

    deleteAndAnalyseRepeated(marcoNuevo -> nro_marco);

    dictionary_put(rowTLB,  pagina, marcoNuevo);
    log_info(logger_tlb, "SE AGREGO LA PAGINA:  %s", pagina);
    list_add(listSeguridadTLB,pagina);
    return NULL;
}