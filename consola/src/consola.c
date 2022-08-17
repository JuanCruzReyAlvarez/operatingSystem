#include "consola.h"

void agregarNuevaInstruccionALaLista(char* linea, t_list* listaInstrucciones){
    char** instruccionDividida = string_split(linea, " ");
    int cantParametros = cantidadParametros(linea);
    
    if (cantParametros == -1){
        
        exit(1); 
    }
    instruccion *nuevaInst = malloc(sizeof(instruccion));
    nuevaInst -> id_INSTRUCCION = instruccionDividida[0];
    if (strcmp(nuevaInst->id_INSTRUCCION, "NO_OP") != 0) {            
        for (int i=0; i<cantParametros; i++){
            nuevaInst -> param[i] = (uint32_t)atoi(instruccionDividida[i+1]);
        }
    list_add(listaInstrucciones,nuevaInst);
    } 
    else if (strcmp(nuevaInst->id_INSTRUCCION, "NO_OP") == 0) {     
        for (int j=0;j<atoi(instruccionDividida[1]); j++) {
            list_add(listaInstrucciones, nuevaInst);
        }
    }
    
    return;
}

bool generar_conexiones(t_log* logger, int* fd_kernel) {
    char* ip_kernel;
    char* puerto_kernel;

    t_config* nuevo_config;
	nuevo_config = config_create("consola.config"); 
    ip_kernel = config_get_string_value(nuevo_config,"IP_KERNEL");
    puerto_kernel = config_get_string_value(nuevo_config,"PUERTO_KERNEL");

    *fd_kernel = crear_conexion(logger,"KERNEL",ip_kernel, puerto_kernel); 

    config_destroy(nuevo_config);

    return *fd_kernel != 0;
}

t_list* agregarInstrucciones(t_list* listaInstrucciones, FILE *archivoInstrucciones){

    if (archivoInstrucciones == NULL){
        printf("archivo sin instrucciones");
        exit(1);
    }
    else{
        char* caracter = malloc(sizeof(char));
        fread(caracter,sizeof(char),1,archivoInstrucciones);
        
        while(!feof(archivoInstrucciones)){
            char* linea = string_new(); 

            while((*caracter)!='\n' && !feof(archivoInstrucciones)){
                string_append(&linea, caracter);    
                fread(caracter,sizeof(char),1,archivoInstrucciones);
            }
            
            agregarNuevaInstruccionALaLista(linea, listaInstrucciones);
        
            fread(caracter,sizeof(char),1,archivoInstrucciones);
            
            free(linea);
        }
        free(caracter);
    }
    return listaInstrucciones;
}

void destruirListaInstrucciones(t_list* lista) {
    while(!list_is_empty(lista)) {
        instruccion* inst = list_remove(lista, 0);
        if (inst -> id_INSTRUCCION == NULL) {
            free(inst);
        }
        else if (strcmp(inst -> id_INSTRUCCION, "NO_OP") == 0) {
            free(inst -> id_INSTRUCCION);
            free(inst);
        } 
        else if (cantidadParametros(inst -> id_INSTRUCCION) != -1) {
            free(inst -> id_INSTRUCCION);
            free(inst);
        }
    }
    free(lista);
}

int main(int argc, char** argv){

    if(argc < 2) {
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    uint32_t tamanioProceso = (uint32_t)atoi(argv[2]);   

    printf("EL PID DE ESTA CONSOLA ES %d\n", getpid());

    //printf("EL tamanio en consola del proceso es %d \n",tamanioProceso);

    FILE *archivoInstrucciones; 
    archivoInstrucciones = fopen(path,"r");

    t_list* listaInstrucciones = list_create();

    listaInstrucciones = agregarInstrucciones(listaInstrucciones,archivoInstrucciones);
    

    char* mensajeRecibido;

    // CONEXIONES
    t_log* logger = log_create("consola.log", "CONSOLA", 1, LOG_LEVEL_INFO);

    fd_consola_a_kernel=0;
    if (!generar_conexiones(logger, &fd_consola_a_kernel)) {                                      
        log_destroy(logger);
        return EXIT_FAILURE;
    }
    
    if(!send_lista_instrucciones(fd_consola_a_kernel,listaInstrucciones,tamanioProceso))
        log_error(logger,"No pudo mandar las instrucciones, algo anda mal!");
    else
        log_info(logger,"Se mandaron bien las instrucciones a Kernel");
   

    if(!recv_mensaje_consola(fd_consola_a_kernel,&mensajeRecibido))
        log_error(logger,"NO SE PUDO RECIBIR UN MENSAJE DE FIN, FALLO , algo anda mal!");
    else
        log_info(logger,"SE RECIBIO EL MENSAJE: %s",mensajeRecibido);

    
    
    
    

    fclose(archivoInstrucciones);
    
    destruirListaInstrucciones(listaInstrucciones);
    log_destroy(logger);
    
    
}
