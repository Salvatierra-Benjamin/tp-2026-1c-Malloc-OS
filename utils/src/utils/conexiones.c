#define _POSIX_C_SOURCE 200112L // Habilita POSIX 2001 para addrinfo
#include "conexiones.h"

// int iniciar_servidor(char* PUERTO)
// {
// 	int socket_servidor = -1;

// 	struct addrinfo hints, *servinfo;

// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;

// 	getaddrinfo(NULL, PUERTO, &hints, &servinfo);

// 	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
// 	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

// 	listen(socket_servidor, SOMAXCONN);

// 	freeaddrinfo(servinfo);

// 	return socket_servidor;
// }

// int esperar_cliente(int socket_servidor)
// {
// 	int socket_cliente = accept(socket_servidor, NULL, NULL);
// 	return socket_cliente;
// }
// int crear_conexion(char* ip, char* puerto)
// {
//     struct addrinfo hints;
//     struct addrinfo *server_info;

//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_UNSPEC;y
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     getaddrinfo(ip, puerto, &hints, &server_info);

//     // Ahora vamos a crear el socket.
//     int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

//     // Ahora que tenemos el socket, vamos a conectarlo
//     int err;
//     err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
//     if (err != 0)
// 	{

// 		socket_cliente = -1;
// 	}

//     freeaddrinfo(server_info);

//     return socket_cliente;
// }

int iniciar_servidor(char *PUERTO)
{
	int socket_servidor = -1;
	struct addrinfo hints, *servinfo;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, PUERTO, &hints, &servinfo);
	if (err != 0)
	{
		fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(err));
		return -1;
	}

	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (socket_servidor == -1)
	{
		perror("Error al crear el socket del servidor");
		freeaddrinfo(servinfo);
		return -1;
	}

	// --- MAGIA CONTRA EL TIME_WAIT ---
	int activado = 1;
	if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)) == -1)
	{
		perror("Error al configurar SO_REUSEADDR");
		close(socket_servidor);
		freeaddrinfo(servinfo);
		return -1;
	}
	// ---------------------------------

	if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		perror("Error al hacer bind en el servidor");
		close(socket_servidor);
		freeaddrinfo(servinfo);
		return -1;
	}

	if (listen(socket_servidor, SOMAXCONN) == -1)
	{
		perror("Error al poner el socket a escuchar (listen)");
		close(socket_servidor);
		freeaddrinfo(servinfo);
		return -1;
	}

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	if (socket_cliente == -1)
	{
		perror("Error al aceptar una nueva conexión de cliente");
	}
	return socket_cliente;
}

int crear_conexion(char *ip, char *puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0; // AI_PASSIVE es solo para el servidor, en el cliente va en 0

	err = getaddrinfo(ip, puerto, &hints, &server_info);
	if (err != 0)
	{
		fprintf(stderr, "Error en getaddrinfo del cliente: %s\n", gai_strerror(err));
		return -1;
	}

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (socket_cliente == -1)
	{
		perror("Error al crear el socket del cliente");
		freeaddrinfo(server_info);
		return -1;
	}

	// Ahora que tenemos el socket, vamos a conectarlo
	err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	if (err == -1)
	{
		perror("Error al intentar conectar con el servidor");
		close(socket_cliente); // Importantísimo cerrar el socket si falla la conexión
		socket_cliente = -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}
