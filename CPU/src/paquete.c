#include "paquete.h"

t_envio* enviar_y_esperar_respuesta_con_paquete(int socket,
		t_envio* paquete, int codOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)()) {
	enviar(socket, paquete, executeIfConnectionError);
	t_envio* nuevo_paquete = recibir(socket,
			executeIfConnectionError);
	if (nuevo_paquete->codigo_Operacion == codOpProblematico)
		executeIfProblem();
	return nuevo_paquete;
}

t_envio* enviar_y_esperar_respuesta(int socket, void* datos,
		int codigo_Operacion, t_size size, int codigoOpProblematico,
		void (*executeIfConnectionError)(), void (*executeIfProblem)()) {
	t_envio* paquete = pedirPaquete((void*) datos, codigo_Operacion, size);
	t_envio* nuevo_paquete = enviar_y_esperar_respuesta_con_paquete(
			socket, paquete, codigoOpProblematico, executeIfConnectionError,
			executeIfProblem);
	free(paquete);
	return nuevo_paquete;
}

t_envio* ecribirEnUMC(int pagina, t_puntero_instruccion offset, t_size tamano, void* data) {
	/*t_direccion* direccion = mallocCommon(sizeof (t_direccion));
	direccion->pagina = pagina;
	direccion->offset = offset;
	direccion->size = dataSize;*/
	t_envio* paquete = pedirPaquete(&pagina, CPU_UMC_Escribir,sizeof(int));
	aniadirAlPaquete(paquete,&offset,sizeof(t_puntero_instruccion));
	aniadirAlPaquete(paquete,&tamano,sizeof(t_size));
	aniadirAlPaquete(paquete,data,tamano);
	return paquete;
}


t_envio* paquete_leerUMC(int pagina, t_puntero_instruccion offset, t_size tamano) {
	/*t_direccion *data = mallocCommon(sizeof(t_direccion));
	data->pagina = pagina;
	data->offset = offset;
	data->size = dataSize;*/
	t_envio* paquete = pedirPaquete(&pagina, CPU_UMC_Leer, sizeof(int));
	aniadirAlPaquete(paquete,&offset,sizeof(t_puntero_instruccion));
	aniadirAlPaquete(paquete,&tamano,sizeof(t_size));
	//free(data);
	return paquete;
}

t_envio* paquete_asignarCompartida(t_nombre_compartida variable, t_valor_variable valor) {
	t_envio* paquete = pedirPaquete((void*) variable,
	CPU_Nucleo_AsignarVariableCompartida, string_length(variable));
	aniadirAlPaquete(paquete, &valor, sizeof(t_valor_variable));
	return paquete;
}

t_envio* solicitar_algo_UMC(int pagina, t_puntero_instruccion start, t_size offset) {
	t_envio* paquete = paquete_leerUMC(pagina, start, offset);
	enviar(umcSocket, paquete, NULL);
	destruirPaquete(paquete);
	paquete = recibir(umcSocket, NULL);
	if (paquete->codigo_Operacion != CPU_UMC_Leer) {
		log_error(logger,"Hubo un error de lectura, cierro la ejecución del programa");
		ejecutarSiFalloMemoria();
	} else {
		return paquete;
	}
	destruirPaquete(paquete);
	return NULL;
}

