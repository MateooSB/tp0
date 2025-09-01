#include "client.h"

// Variable global para compartir las líneas leídas
t_list* lineas_consola = NULL;
t_log* logger = NULL;

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

	int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_config* config;

	/* ---------------- LOGGING ---------------- */
	
	logger = iniciar_logger();

	// Usando el logger creado previamente
	// Escribi: "Hola! Soy un log"
	log_info(logger, "Soy un Log");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config();

	// Usando el config creado previamente, leemos los valores del config y los 
	// dejamos en las variables 'ip', 'puerto' y 'valor'
	valor = config_get_string_value(config, "CLAVE");
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	// Loggeamos el valor de config
	log_info(logger, "Leí la config, donde CLAVE=%s", valor);

	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	// ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

	// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	nuevo_logger = log_create("tp0.log", "Cliente", true, LOG_LEVEL_INFO);
	
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config;
	
	nuevo_config = config_create("cliente.config");
	
	if (nuevo_config == NULL) {
		printf("Error: No se pudo leer el archivo de configuración cliente.config\n");
		abort();
	}
	
	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;

	log_info(logger, "Iniciando lectura de consola...");

	// La primera te la dejo de yapa
	leido = readline("> ");

	// Verificamos que readline no haya fallado
	if (leido == NULL) {
		log_error(logger, "Error al leer de consola");
		return;
	}

	// Logueamos la primera línea leída
	log_info(logger, "Leí de consola: %s", leido);

	// Si es una línea vacía, terminamos inmediatamente
	if (string_is_empty(leido)) {
		log_info(logger, "Primera línea vacía. Terminando lectura de consola.");
		free(leido);
		return;
	}
	
	// Inicializamos la lista global solo si tenemos al menos una línea
	lineas_consola = list_create();
	if (lineas_consola == NULL) {
		log_error(logger, "Error al crear lista de líneas");
		free(leido);
		return;
	}

	// Agregamos la primera línea a la lista
	list_add(lineas_consola, leido);

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(1) {
			leido = readline("> ");

			if (leido == NULL) {
				log_error(logger, "Error al leer de consola");
				break;
			}
			
			if (string_is_empty(leido)) {
				log_info(logger, "Se ingresó una línea vacía. Terminando lectura de consola.");
				free(leido);
				break;
			}
			
			log_info(logger, "Leí de consola: %s", leido);
			
			list_add(lineas_consola, leido);
		}
	
	log_info(logger, "Terminé de leer de consola");
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	// Verificamos que tengamos líneas para enviar
	if (lineas_consola == NULL || list_is_empty(lineas_consola)) {
		log_info(logger, "No hay líneas para enviar en el paquete");
		return;
	}

	log_info(logger, "Creando paquete con %d líneas", list_size(lineas_consola));

	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete
	for(int i = 0; i < list_size(lineas_consola); i++) {
		leido = (char*) list_get(lineas_consola, i);
		log_info(logger, "Agregando al paquete: %s", leido);
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	}

	// Enviamos el paquete
	log_info(logger, "Enviando paquete...");
	enviar_paquete(paquete, conexion);

	// Liberamos el paquete
	eliminar_paquete(paquete);

	// Y liberamos las líneas de la lista
	list_destroy_and_destroy_elements(lineas_consola, free);
	lineas_consola = NULL;
	
	log_info(logger, "Paquete enviado y memoria liberada");
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
	liberar_conexion(conexion);
	config_destroy(config);
	log_destroy(logger);
}
