#include "stack.h"

void pushStack(t_stack* stack, t_nombre_variable dato, int umcSocket) {

log_info(logger, "pusheo");
	t_intructions direccion;
		direccion.start = (stack->stackPointer)->start;

		direccion.offset = (stack->stackPointer)->offset;
		t_puntero posicionSP = calculoDeDireccionAPosicion(direccion);
//log_info(logger,"Pagina: %d offset:%d", stack->stackPointer->start, stack->stackPointer->offset);
//log_info(logger, "posicionSPactual %d, ",posicionSP);

	//controlo no pasarme de las paginas qe tengo
	//	if(posicionSP + tamanoVariable <= calculoStackOverFlow(stack->base->start, stack->size)){
if((stack->stackPointer->offset + 4 > tamanoPagina) &&
		(stack->stackPointer->start - stack->base->start == stack->size -1)){


//log_info(logger,"stack over flow");
ejecutarSiHuboStackOverflow();
//cambiarEstado(STK_OVERFLOW);

}if (estado != STK_OVERFLOW){
			t_envio* datos = ecribirEnUMC(stack->stackPointer->start,
					stack->stackPointer->offset ,tamanoVariable, "");
	//		log_info(logger,"Push de %c", dato);
	//		log_info(logger,"Pagina: %d offset:%d", stack->stackPointer->start, stack->stackPointer->offset);
			//guardo lugar para ese dato
			t_envio* respuesta = enviar_y_esperar_respuesta_con_paquete(umcSocket,
			datos, UMC_FalloMemoria, ejecutarSiSePerdioConexion,ejecutarSiHuboStackOverflow);

			//guardo en diccionario la ubicacion de este dato - como una posicion
				t_intructions valor;
				valor.start=stack->stackPointer->start ;
				valor.offset=stack->stackPointer->offset ;
				t_puntero info = calculoDeDireccionAPosicion(valor);
				char* key = charAString(dato);
			dictionary_put(stack->diccionarioDeVariables, key, info);

	//		log_info(logger,"Push - La guardo en %u", info);

			destruirPaquete(datos);
			destruirPaquete(respuesta);

			posicionSP = posicionSP + tamanoVariable;
			t_intructions nuevaDireccion= calculoDePosicionADireccion(posicionSP);
			stack->stackPointer->start = nuevaDireccion.start;
			stack->stackPointer->offset = nuevaDireccion.offset;
			log_info(logger,"Push - aumento StackPointer %d", posicionSP);
			log_info(logger,"Push - aumento StackPointer pag %d, off %d", nuevaDireccion.start,nuevaDireccion.offset);

}
	}


t_puntero ultimaVariable(t_stack* stack) {
	t_intructions valorX;
	stack->stackPointer->start = valorX.start;
	stack->stackPointer->offset = valorX.offset;

	t_puntero valor = calculoDeDireccionAPosicion(valorX);
	return valor - tamanoVariable;
}



