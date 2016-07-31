#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_


#include <commons/log.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "protocolo.h"
#include "estructuras.h"
#include "terminaciones.h"
#include <parser/parser.h>
#include <commons/collections/dictionary.h>
#include <unistd.h>
#include "CPU.h"
#include "common_sockets.h"


typedef struct {
	char* ipNucleo;
	int puertoNucleo;
	char* ipUMC;
	int puertoUMC;
} t_configCPU;

t_config *configCPU;

void inicializarLog();
void cargarSenales();
void crearConfiguracion(char* configPath);
t_PCB* obtenerPCB();
void ejecutarInstruccion(int QUANTUM_SLEEP);
void avisarProcesoActivo();
t_datosNucleo ConectarConNucleo();
t_datosNucleo handshakeNucleo();
t_datosNucleo obtenerDataNuleo(void* data);
t_datosUMC conectarConUMC();
t_datosUMC handshakeUMC();
void destruirConexion();
t_datosUMC obtenerPagina(void* data);
void terminarProceso(void* datosExtra, t_size sizeExtra);
void acciones();
t_puntero calculoDeDireccionAPosicion(t_intructions direccion);
t_intructions calculoDePosicionADireccion(t_puntero posicion);
void ejecutarSiHuboStackOverflow();
void ejecutarSiFalloMemoria();
bool estadoDesconexion(state estado);
void cambiarEstado(state toState);
void ejecutarSiSePerdioConexion();
void noHacerNada();
void senalDesconexion(int signal);
char* charAString(char element);
t_PCB* desempaquetarPCB(void* data);

#endif /* FUNCIONESCPU_H_ */
