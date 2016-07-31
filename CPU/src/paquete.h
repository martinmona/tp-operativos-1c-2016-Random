#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <parser/parser.h>

#include "common_sockets.h"
#include "protocolo.h"
#include "estructuras.h"
#include "CPU.h"


t_envio* enviar_y_esperar_respuesta_con_paquete(int socket,
		t_envio* paquete, int codOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)());
		
t_envio* enviar_y_esperar_respuesta(int socket, void* datos,
		int codigoOperacion, t_size size, int codigoOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)());
		
t_envio* ecribirEnUMC(int pagina, t_puntero_instruccion offset, t_size tamano, void* data);
		
t_envio* paquete_leerUMC(int pagina, t_puntero_instruccion offset, t_size tamano);

t_envio* paquete_asignarCompartida(t_nombre_compartida variable, t_valor_variable valor);

t_envio* solicitar_algo_UMC(int pagina, t_puntero_instruccion start, t_size offset) ;





#endif /* PAQUETE_H_ */
