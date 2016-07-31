#ifndef LIBRERIACOMUN_PROTOCOLO_H_
#define LIBRERIACOMUN_PROTOCOLO_H_


#define CPU_Nucleo_Handshake 1

#define CPU_UMC_Handshake 20
#define NoHayErrorPosible 3

#define CPU_UMC_Leer 26
#define CPU_UMC_Escribir 28
#define CPU_UMC_ProcesoActivo 23
#define CPU_UMC_Ok 21

#define UMC_FalloMemoria 25
#define CPU_UMC_Cae 19

#define CPU_Nucleo_PCB 90
#define CPU_Nucleo_ProgramaAEjecutar 9
#define CPU_Nucleo_Ok 10
#define CPU_Nucleo_AsignarVariableCompartida 11
#define CPU_Nucleo_PedirVariableCompartida 12
#define CPU_Nucleo_FinDeQuantum 13
#define CPU_Nucleo_Excepcion 14
#define CPU_Nucleo_IO 15
#define CPU_Nucleo_ProcesoEnEspera 16
#define CPU_Nucleo_FinProceso 17
#define CPU_Nucleo_Desconexion 18
#define CPU_Nucleo_Imprimir 20
#define CPU_Nucleo_ImprimirTexto 21
#define CPU_Nucleo_Wait 22
#define CPU_Nucleo_WaitOK 23
#define CPU_Nucleo_WaitNoOk 24
#define CPU_Nucleo_Signal 25

#define RESERVAR_MEMORIA 26
#define EJECUTAR 27
#define FALTA_ESPACIO_SWAP 28

#define CONSOLA_PaqueteParaImprimir 29

#endif /* LIBRERIACOMUN_PROTOCOLO_H_ */
