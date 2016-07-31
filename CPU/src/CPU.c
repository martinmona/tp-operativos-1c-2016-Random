#include "CPU.h"



int main(void) {
	puts("-------------Inicio de proceso CPU----------------");
	inicializarLog();
	PCBActual=mallocCommon((sizeof(t_PCB)));
	stackActual= mallocCommon(sizeof(t_stack));
	//Leo archivo de config
	crearConfiguracion("../resources/CPUconfig.cfg");
	//Cargo señales
	cargarSenales();

	t_datosUMC UMCData = conectarConUMC();
//	tamanoPagina=UMCData.tamanoPagina;  
	t_datosNucleo nucleoData =  ConectarConNucleo();
	//quantum=nucleoData.QUANTUM;
	//QUANTUM_SLEEP=nucleoData.QUANTUM_SLEEP;
	tamanoPagina= UMCData.tamanoPagina;
	acciones();

	estado = NORMAL;

	while (estado != KILL_CPU) {
     estado = NORMAL;
	//Recibe la PCB de un programa
     log_info(logger,"Esperando PCB");
	PCBActual = obtenerPCB();
	stackActual = (t_stack*)list_get(PCBActual->IS,PCBActual->entornoActual);
//	log_info(logger,"Entorno %d sp base %d off %d",PCBActual->entornoActual,stackActual->stackPointer->start,stackActual->stackPointer->offset);
	log_info(logger,"PCBACTUAL: %d",PCBActual->PID);
	avisarProcesoActivo();


//	log_info(logger,"Quantum: %d QuantumSleep: %d",quantum,QUANTUM_SLEEP);
	//while (quantum > 0 && estado != END_PROCESS && estado != ENTRADA_SALIDA && estado != WAIT && estado != FALLO_MEM) {
	while (quantum > 0 && procesoActivo ) {
		log_info(logger,"PC %d",PCBActual->PC);
		//Incremento el Program Counter
		//Verifico el Indice de codigo y pido a UMC proxima sentencia a ejecutar (puede existir excepción)
		//Parseo
		//Ejecuto las operaciones

		ejecutarInstruccion(QUANTUM_SLEEP);
		quantum--;
		log_info(logger,"RESTO QUANTUM. AHORA Quantum: %d",quantum);
	}
	log_info(logger,"Deteniendo ejecucion");
		if (estado == KILL_CPU) {
		log_info(logger,"El proceso CPU se va a desconectar");
		terminarCPU(NULL, 0);
		} else {
		 if (procesoActivo == true) {
		 terminarProceso(NULL, 0);
		 }
		}
	free(PCBActual);
	free(stackActual);
	}
	terminarCPU(NULL,0);
	return EXIT_SUCCESS;
}
