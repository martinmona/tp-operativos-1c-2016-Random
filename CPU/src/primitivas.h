#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include "estructuras.h"
#include "common_sockets.h"
#include "protocolo.h"
#include "paquete.h"
#include "funcionesCPU.h"
#include "stack.h"
#include <parser/metadata_program.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include <stdint.h>
#include <stdlib.h>


t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signal_(t_nombre_semaforo identificador_semaforo);
void finalizar();

#endif /* PRIMITIVAS_H_ */
