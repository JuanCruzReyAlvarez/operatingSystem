#include "shared_utils.h"






char* mi_funcion_compartida(){
    return "Hice uso de la shared!";
}

int cantidadParametros(char* linea){ // Se le pasa una línea del pseudo con las instrucciones y devuelve cuántas instrucciones tiene que leer (o -1 si no reconoce la instrucción)
    char** laInstruccion = string_split(linea, " ");
    char* identificador = laInstruccion[0];
    //printf("\n\n\n\nINSTRUCCION %s\n\n\n\n", identificador);
    if (strcmp(identificador, "EXIT") == 0 || strcmp(identificador, "NO_OP") == 0) { // 0 parámetros
        return 0;
    } else
    if (strcmp(identificador, "I/O") == 0 || strcmp(identificador, "READ") == 0) { // 1 parámetro
        return 1;
    } else
    if (strcmp(identificador, "COPY") == 0 || strcmp(identificador, "WRITE") == 0) { // 2 parámetros
        return 2;
    }

    return -1; // CÓDIGO DE ERROR
}

algoritmoTLB generarEnumAlgoritmo(char *algoritmo){
    if(strcmp(algoritmo,"FIFO") == 0){
        return FIFO;}

        else if(strcmp(algoritmo,"LRU") == 0){
        return LRU;}
        else
        printf("ERROR, RECIBI UN ALGORITMO INEXISTENTE (%s)\n", algoritmo);
        exit(-1);

}

op_code generarCode(char *inst){

        if(strcmp(inst,"NO_OP") == 0){
        return NO_OP;}

        else if(strcmp(inst,"WRITE") == 0){
        return WRITE;}

        else if(strcmp(inst,"READ") == 0){
        return READ;}

        else if(strcmp(inst,"I/O") == 0){
        return IO;}

        else if(strcmp(inst,"COPY") == 0){
        return COPY;}

        else if(strcmp(inst,"EXIT") == 0){
        return EXIT;}

        else
        printf("ERROR, RECIBI UNA INSTRUCCION CON ID INEXISTENTE (%s)\n", inst);
        exit(-1);
    }

uint32_t id_a_indice(uint32_t id, t_list* lista) {  //chequear, lo hizo Copilot
    uint32_t i;
    for(i=0;i<list_size(lista);i++){
        pcb* pcb_actual = list_get(lista,i);
        if(pcb_actual -> id == id){
            return i;
        }
    }
    return -1;
}

char* generarString(op_code inst){
    
        if(inst == NO_OP){
        return "NO_OP";}
        
        else if(inst == WRITE){
        return "WRITE";}

        else if(inst == READ){
        return "READ";}

        else if(inst == IO){
        return "I/O";}

        else if(inst == COPY){
        return "COPY";}

        else{
        return "EXIT";}

    }

char* generar_string_de_estado(estado estado_proceso){
    switch (estado_proceso) {
        case NEW:
            return "NEW";
        case READY:
            return "READY";
        case EXEC:
            return "EXEC";
        case BLOCKED:
            return "BLOCKED";
        case FIN:
            return "FIN";
        case SUSPENDED_BLOCKED:
            return "SUSPENDED_BLOCKED";
        case SUSPENDED_READY:
            return "SUSPENDED_READY";
        default:
            return "ESTADO_DESCONOCIDO";
    }
}

float rafaga_restante(pcb* pcb_rafaga) {
    return pcb_rafaga->estimacion_rafaga;
}