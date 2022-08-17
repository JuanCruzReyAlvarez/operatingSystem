#include "logs_cpu.h"


void cpu_log_inicial(t_log* logger){  
    log_info(logger, "COMIENZO CICLO DE INSTRUCION ----------------------------------");
}


void decode_log(t_log* logger){
	log_info(logger, "DECODE EJECUTADO : Instruccion interpretada. ");
}	

void execute_log(t_log*logger){
	log_info(logger, "EXECUTE EJECUTADO: Instruccion ejecutada  y PCB actualizada.");
}

void fetch_log(t_log* logger){  
    log_info(logger, "FETCH EJECUTADO : Instruccion encontrada y PC actualizado.");
	
}
void fetchOperands_log(t_log* logger){  
    log_info(logger, "FETCH OPERANDS EJECUTADO : Valor a copiar encontrado.");
}
