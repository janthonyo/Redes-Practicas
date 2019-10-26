#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <vector>

#include "funcionesServer.hpp"


#define MSG_SIZE 250
#define MAX_CLIENTS 30
#define MAX_GAMES 10


/*
 * El servidor ofrece el servicio de un chat
 */


void manejador(int signum);
void salirCliente(int socket, fd_set * readfds, int * numClientes, std::vector<cliente> arrayClientes);


int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, new_sd;
	struct sockaddr_in sockname, from;
	char buffer[MSG_SIZE];
	socklen_t from_len;
    fd_set readfds, auxfds;
    int salida;

    /*---------------------------------------------------

        arrayClientes sera un vector de clientes el cual
        contendra:

        - sd: El socket por el que habla el cliente
        - status: El estado actual del cliente
        - user: Una cadena para guardar el user del cliente
        - passwd: Idem para la contraseña

    ---------------------------------------------------*/

    std::vector<cliente> arrayClientes;
    arrayClientes.resize(MAX_CLIENTS);

    std::vector<domino> partida;
    partida.resize(MAX_GAMES);

    std::vector<int> lista_espera;

    int numClientes = 0;
    int numPartidas = 0;
    //contadores
    int i,j,k;
	int recibidos;
    char identificador[MSG_SIZE];
    int contNum = 0;

    // Cadenas auxiliares
    char *aux;
    char *user;
    char *passwd;
    char mensaje[MSG_SIZE];
    int pos, result;

    int on, ret;

    char valorFicha[2];
    char *extremo;
    ficha auxFicha;

	std::string auxstr;	//Para el mensaje de robo de ficha

    /*---------------------------------------------------

        Status nos servira para ver el estado en el que
        se encuentra un jugador.

        Posibles estados:

        0. Esperando usuario
        1. Esperando contraseña
        2. Esperando partida
        3. Esperando turno
        4. Mi turno

    ---------------------------------------------------*/

    int clienteX;

	/* --------------------------------------------------
		Se abre el socket
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (-1);
	}

    // Activaremos una propiedad del socket que permitir· que otros
    // sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    // Esto permitir· en protocolos como el TCP, poder ejecutar un
    // mismo programa varias veces seguidas y enlazarlo siempre al
    // mismo puerto. De lo contrario habrÌa que esperar a que el puerto
    // quedase disponible (TIME_WAIT en el caso de TCP)
    on=1;
    ret = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));



	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2050);
	sockname.sin_addr.s_addr =  INADDR_ANY;

	if (bind (sd, (struct sockaddr *) &sockname, sizeof (sockname)) == -1)
	{
		perror("Error en la operación bind");
		exit(-1);
	}


   	/*---------------------------------------------------------------------
		Del las peticiones que vamos a aceptar sólo necesitamos el
		tamaño de su estructura, el resto de información (familia, puerto,
		ip), nos la proporcionará el método que recibe las peticiones.
   	----------------------------------------------------------------------*/
		from_len = sizeof (from);


		if(listen(sd,1) == -1){
			perror("Error en la operación de listen");
			exit(-1);
		}

    //Inicializar los conjuntos fd_set
    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd,&readfds);
    FD_SET(0,&readfds);


    //Capturamos la señal SIGINT (Ctrl+c)
    signal(SIGINT,manejador);

	/*-----------------------------------------------------------------------
		El servidor acepta una petición
	------------------------------------------------------------------------ */
		while(1){

            //Esperamos recibir mensajes de los clientes (nuevas conexiones o mensajes de los clientes ya conectados)

            auxfds = readfds;

            salida = select(FD_SETSIZE,&auxfds,NULL,NULL,NULL);

            if(salida > 0){


                for(i=0; i<FD_SETSIZE; i++){

                    //Buscamos el socket por el que se ha establecido la comunicación
                    if(FD_ISSET(i, &auxfds)) {

                        if( i == sd){

                            if((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
                                perror("Error aceptando peticiones");
                            }
                            else
                            {
                                if(numClientes < MAX_CLIENTS){
                                    arrayClientes[numClientes].sd = new_sd;
                                    arrayClientes[numClientes].status = 0;
                                    arrayClientes[numClientes].inGame = -1;
                                    numClientes++;
                                    FD_SET(new_sd,&readfds);

                                    bzero(buffer,sizeof(buffer));
                                    strcpy(buffer, "+0k. Usuario conectado.\n");
                                    send(new_sd,buffer,strlen(buffer),0);
                                }
                                else
                                {
                                    bzero(buffer,sizeof(buffer));
                                    strcpy(buffer,"-ERR. Demasiados clientes conectados\n");
                                    send(new_sd,buffer,strlen(buffer),0);
                                    close(new_sd);
                                }

                            }


                        }
                        else if (i == 0){
                            //Se ha introducido información de teclado
                            bzero(buffer, sizeof(buffer));
                            fgets(buffer, sizeof(buffer),stdin);

                            //Controlar si se ha introducido "SALIR", cerrando todos los sockets y finalmente saliendo del servidor. (implementar)
                            if(strcmp(buffer,"SALIR\n") == 0){

                                for (j = 0; j < numClientes; j++){
                                    send(arrayClientes[j].sd, "-ERR. Desconexion servidor\n", strlen("-ERR. Desconexion servidor\n"),0);
                                    close(arrayClientes[j].sd);
                                    FD_CLR(arrayClientes[j].sd,&readfds);
                                }
                                    close(sd);
                                    exit(-1);


                            }
                            //Mensajes que se quieran mandar a los clientes (implementar)

                        }
                        else{
                            bzero(buffer,sizeof(buffer));

                            recibidos = recv(i,buffer,sizeof(buffer),0);

                            if(recibidos > 0){

                                    for(j=0; j<numClientes; j++)
                                        if(arrayClientes[j].sd == i)
                                        {
                                            clienteX = j;
                                            //sprintf(identificador,"%d: %s (Status: %d)\n", i, buffer, arrayClientes[j].status);
                                        }

                                    //bzero(buffer,sizeof(buffer));
                                    //strcpy(buffer,identificador);
                                    //send(i,buffer,strlen(buffer),0);

                                    switch(arrayClientes[clienteX].status)
                                    {
                                        //login / registro
                                        case 0:                 // TERMINADO
                                            if( strstr(buffer, "USUARIO " ) != NULL )
                                            {
                                                // Sacamos la cadena USUARIO
                                                aux = strtok(buffer, " ");

                                                // Guardamos el nombre del usuario
                                                aux = strtok(NULL, "\n");

                                                // El usuario aparece en el fichero de datos
                                                if(checkUser(aux))
                                                {
                                                    // Guardamos el user del cliente.
                                                    strcpy(arrayClientes[clienteX].user, aux);

                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "+Ok. Usuario correcto\n");
                                                    send(i, buffer, strlen(buffer), 0);

                                                    // Actulizamos el estado del cliente.
                                                    arrayClientes[clienteX].status = 1;
                                                }

                                                // El usuario NO aparece en el fichero de datos
                                                else
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Usuario incorrecto\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }

                                            }

                                            else if ( strstr(buffer, "REGISTRO " ) != NULL )
                                            {
                                                aux = strtok(buffer, " ");

                                                // Sacamos el user y la passwd del buffer
                                                while( aux != NULL )
                                                {
                                                    if (strcmp( aux, "-u" ) == 0 )
                                                        user = strtok(NULL, " ");

                                                    if (strcmp( aux, "-p") == 0 )
                                                        passwd = strtok(NULL, " ");

                                                    aux = strtok(NULL, " ");
                                                }

                                                // Si algun dato es nulo, faltaban datos en la peticion
                                                if ((user == NULL) or (passwd == NULL))
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Faltan datos para el registro\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }

                                                // El registro es correcto.
                                                else if(registerUser(user, passwd))
                                                {
                                                    // Guardamos el user.
                                                    strcpy(arrayClientes[clienteX].user, user);

                                                    // Guardamos la password.
                                                    strcpy(arrayClientes[clienteX].passwd, passwd);

                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "+Ok. Usuario registrado correctamente\n");
                                                    send(i, buffer, strlen(buffer), 0);
													//Se informa al servidor también
													printf("+Ok. Nuevo usuario registrado\n");

                                                    arrayClientes[clienteX].status = 2;
                                                }

                                                // Hay un usuario con el mismo nombre en el sistema.
                                                else
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. El nombre de usuario ya existe en el sistema.\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }
                                            }

                                            else if(strcmp(buffer,"SALIR\n") == 0)
                                            {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "+Ok. Desconexión procesada\n");
                                                    send(i, buffer, strlen(buffer), 0);

                                                    salirCliente(i,&readfds,&numClientes,arrayClientes);
                                            }

                                            else
                                            {
                                                // Mandamos un mensaje al usuario para recordarle que hace en este
                                                // estado.

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "> Para poder participar, inicie sesion (USUARIO <user-name>)\n  o registrese (REGISTRO -u <user-name> -p <password>)\n");
                                                send(i, buffer, strlen(buffer), 0);
                                            }
                                        break;

                                        // Password
                                        case 1:                 // TERMINADO
                                            if( strstr(buffer, "PASSWORD " ) != NULL )
                                            {
                                                // Sacamos la cadena PASSWORD
                                                aux = strtok(buffer, " ");

                                                // Guardamos la password del cliente
                                                aux = strtok(NULL, "\n");

                                                if( verifyPassword(arrayClientes[clienteX].user, aux))
                                                {
                                                    // Guardamos la passwd del cliente

                                                    strcpy(arrayClientes[clienteX].passwd, aux);

                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "+Ok. Usuario validado.\n");
                                                    send(i, buffer, strlen(buffer), 0);
													//Se informa al servidor también
													printf("+Ok. Inicio de sesión exitoso.\n");

                                                    //Actualizamos su estado
                                                    arrayClientes[clienteX].status = 2;
                                                }

                                                else
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Error en la validación.\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }
                                            }

                                            else if(strcmp(buffer,"SALIR\n") == 0)
                                            {

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. Desconexión procesada.\n");
                                                send(i, buffer, strlen(buffer), 0);

                                                salirCliente(i,&readfds,&numClientes,arrayClientes);

                                            }

                                            else
                                            {
                                                // Mandamos un mensaje al usuario para recordarle que hace en este
                                                // estado.

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "> Introduzca la contraseña (PASSWORD <password>).\n");
                                                send(i, buffer, strlen(buffer), 0);
                                            }
                                        break;

                                        // Iniciar partida
                                        case 2:                 // TERMINADO
                                            if(strcmp(buffer,"INICIAR-PARTIDA\n") == 0)
                                            {
                                                // Comprobamos si existen partidas libres.
                                                if (numPartidas < MAX_GAMES)
                                                {
                                                    // matchmaking devuelve que partida se le ha asignado al cliente.
                                                    // Si no hay ninguna, devuelve -1

                                                    // Almacenamos en una variable la partida asignada al jugador.
                                                    // Si no hay partidas disponible se manda un mensaje de error.
                                                    pos = matchmaking(partida);

                                                    // pos = -1: -> Indica que no hay partidas disponibles actualmente
                                                    if (pos == -1)
                                                    {
                                                        bzero(buffer, sizeof(buffer));
                                                        strcpy(buffer, "-ERR. No hay partidas disponibles.\n");
                                                        send(i, buffer, strlen(buffer), 0);

                                                        // QUEDA: controlar el orden de entrada de los jugadores que no
                                                        //        pueden entrar de momento

                                                        // Guardamos en lista de espera el socket del usuario
                                                        if(!userInWaitList(i, lista_espera))
                                                        {
                                                            lista_espera.push_back(i);
                                                        }
                                                    }

                                                    // pos = x: -> Indica cual es la partida asignada al jugador (x)
                                                    else
                                                    {
                                                        // Guardamos que partida se ha asignado al jugador
                                                        arrayClientes[clienteX].inGame = pos;

                                                        // Comprobamos que jugador falta en partida y lo establecemos.
                                                        if (!partida[pos].hasPlayer1())
                                                        {
                                                            std::cout << "1\n";
                                                            partida[pos].setPlayer1(arrayClientes[clienteX].user);
                                                            partida[pos].setSocket1(arrayClientes[clienteX].sd);

                                                        }

                                                        else
                                                        {
                                                            std::cout << "2\n";
                                                            partida[pos].setPlayer2(arrayClientes[clienteX].user);
                                                            partida[pos].setSocket2(arrayClientes[clienteX].sd);

                                                        }

                                                        // Comprobamos si los dos jugadores estan establecidos para
                                                        // iniciar la partida

                                                        // Falta un jugador
                                                        if (!partida[pos].has2players())
                                                        {
                                                            bzero(buffer, sizeof(buffer));
                                                            strcpy(buffer, "+Ok. Petición Recibida. Quedamos a la espera de más jugadores.\n");
                                                            send(i, buffer, strlen(buffer), 0);
                                                        }

                                                        // Los dos jugadores estan en la partida.
                                                        else
                                                        {
                                                            bzero(buffer, sizeof(buffer));
                                                            strcpy(buffer, "+Ok. Empieza la partida.\n");
                                                            send(i, buffer, strlen(buffer), 0);
															//Se informa al servidor también
															printf("+Ok. Empieza una nueva partida.\n");

                                                            numPartidas++;

                                                            // Comienza la partida.

                                                            // Establecemos las manos de cada jugador
                                                            partida[pos].setHand1(partida[pos].newHand());
                                                            partida[pos].setHand2(partida[pos].newHand());

                                                            // Vemos quien sale

                                                            // Para ello, comprobamos quien tiene la ficha doble
                                                            // mas alta o, en su defecto, la ficha mas alta.

                                                            // startPlayer() examina los mazos de los jugadores
                                                            // y coloca la ficha doble mas alta o, en su defecto,
                                                            // la ficha mas alta para salir. Esta funcion devuelve
                                                            // que jugador seria el segundo en poner despues de colocar
                                                            // la ficha inicial.

                                                            result = partida[pos].startPlayer();

                                                            // Turno del J1 / Espera J2
                                                            if (result == 1)
                                                            {
																//Enviamos los jugadores
																strcpy(mensaje, "\nJUGADORES:\n");
																strcat(mensaje, "\tJugador1(");
																strcat(mensaje, partida[pos].getUserP1());
                                                                strcat(mensaje, ")\n");
																strcat(mensaje, "\tJugador2(");
																strcat(mensaje, partida[pos].getUserP2());
                                                                strcat(mensaje, ")\n\n");
																send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

																//Enviamos que jugador ha sacado.
                                                                strcpy(mensaje, "+Ok. Jugador2 sale y ha colocado ficha.\n\n");
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                // Enviamos los turnos
                                                                bzero(buffer, sizeof(buffer));
                                                                strcpy(buffer, "+Ok. Turno de partida.\n");
                                                                send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);

                                                                bzero(buffer, sizeof(buffer));
                                                                strcpy(buffer, "+Ok. Turno del otro jugador.\n");
                                                                send(partida[pos].getSocketP2(), buffer, strlen(buffer), 0);

                                                                // Enviamos el tablero
                                                                strcpy(mensaje, partida[pos].showBoard().c_str());
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                // Send hands
                                                                strcpy(mensaje, partida[pos].messageHandP1().c_str());
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);

                                                                strcpy(mensaje, partida[pos].messageHandP2().c_str());
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                // Cambiamos los estados de los jugadores
                                                                for(j = 0; j < numClientes; j++)
                                                                {
                                                                    if (arrayClientes[j].sd == partida[pos].getSocketP1())
                                                                        arrayClientes[j].status = 4;        // J1 Puede colocar

                                                                    else if (arrayClientes[j].sd == partida[pos].getSocketP2())
                                                                        arrayClientes[j].status = 3;        // J2 espera
                                                                }


                                                            }

                                                            // Turno de J2 / Espera J1
                                                            else
                                                            {
																//Enviamos los jugadores
																strcpy(mensaje, "\nJUGADORES:\n");
																strcat(mensaje, "\tJugador1(");
																strcat(mensaje, partida[pos].getUserP1());
                                                                strcat(mensaje, ")\n");
																strcat(mensaje, "\tJugador2(");
																strcat(mensaje, partida[pos].getUserP2());
                                                                strcat(mensaje, ")\n\n");
																send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                //Enviamos que jugador ha sacado.
                                                                strcpy(mensaje, "+Ok. Jugador1 sale y ha colocado ficha.\n\n");
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);


                                                                // Enviamos los turnos
                                                                bzero(buffer, sizeof(buffer));
                                                                strcpy(buffer, "+Ok. Turno del otro jugador.\n");
                                                                send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);

                                                                bzero(buffer, sizeof(buffer));
                                                                strcpy(buffer, "+Ok. Turno de partida.\n");
                                                                send(partida[pos].getSocketP2(), buffer, strlen(buffer), 0);

                                                                // Enviamos el tablero
                                                                strcpy(mensaje, partida[pos].showBoard().c_str());
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                // Send hands
                                                                strcpy(mensaje, partida[pos].messageHandP1().c_str());
                                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);

                                                                strcpy(mensaje, partida[pos].messageHandP2().c_str());
                                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                                // Cambiamos los estados de los jugadores
                                                                for(j = 0; j < numClientes; j++)
                                                                {
                                                                    if (arrayClientes[j].sd == partida[pos].getSocketP1())
                                                                        arrayClientes[j].status = 3;    // J1 Espera

                                                                    else if (arrayClientes[j].sd == partida[pos].getSocketP2())
                                                                        arrayClientes[j].status = 4;    // J2 Puede colocar
                                                                }
                                                            }
                                                        }
                                                    }
                                                }

                                                else
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. No hay partidas disponibles.\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }
                                            }

                                            else if(strcmp(buffer,"SALIR\n") == 0)
                                            {
                                                // El usuario tiene asignada una partida
                                                if(arrayClientes[clienteX].inGame != -1)
                                                {
                                                    // Sacamos que partida tiene asignada
                                                    pos = arrayClientes[clienteX].inGame;

                                                    // Si esta en el estado 2 y ha iniciado partida, solo
                                                    // podra haber un jugador en la partida. Por lo que miramos
                                                    // si se trata del jugador 1 o del 2.

                                                    if(partida[pos].hasPlayer1())
                                                    {
                                                        partida[pos].setSocket1(-1);
                                                    }

                                                    else
                                                    {
                                                        partida[pos].setSocket2(-1);
                                                    }
                                                }

                                                // El usuario todavia no ha iniciado partida o ya no la tiene

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. Desconexión procesada\n");
                                                send(i, buffer, strlen(buffer), 0);

                                                salirCliente(i,&readfds,&numClientes,arrayClientes);
                                            }

                                            else
                                            {
                                                // Mandamos un mensaje al usuario para recordarle que hace en este
                                                // estado.

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "> Introduzca INICIAR-PARTIDA para comenzar.\n");
                                                send(i, buffer, strlen(buffer), 0);
                                            }
                                        break;

                                        // Esperando turno
                                        case 3:                 // CREO QUE ACABADO
                                            if(strstr(buffer, "SALIR\n") != NULL)
                                            {
                                                // Obtenemos la partida en la que jugaba el cliente
                                                pos = arrayClientes[clienteX].inGame;

                                                // Mandamos el mensaje de partida anulada a los jugadores
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. La partida ha sido anulada.\n\n");
                                                send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);
                                                send(partida[pos].getSocketP2(), buffer, strlen(buffer), 0);

                                                // Comprobamos cual es el jugador que NO sale
                                                if(arrayClientes[clienteX].sd == partida[pos].getSocketP1())
                                                {
                                                    //std::cout << "Sale J1\n";
                                                    for( j = 0; j < numClientes; j++)
                                                    {
                                                        if(arrayClientes[j].sd == partida[pos].getSocketP2())
                                                        {
                                                            // Cambiamos el estado para que pueda iniciar otra partida
                                                            //std::cout << "Cambio estado de J2\n";
                                                            arrayClientes[j].status = 2;
                                                        }
                                                    }
                                                }

                                                else if(arrayClientes[clienteX].sd == partida[pos].getSocketP2())
                                                {
                                                    //std::cout << "Sale J2\n";
                                                    for( j = 0; j < numClientes; j++)
                                                    {
                                                        if(arrayClientes[j].sd == partida[pos].getSocketP1())
                                                        {
                                                            // Cambiamos el estado para que pueda iniciar otra partida
                                                            //std::cout << "Cambio estado de J1\n";
                                                            arrayClientes[j].status = 2;

                                                        }
                                                    }
                                                }

                                                else
                                                {
                                                    std::cout << "Esto no deberia de pasar....\n";
                                                }

                                                // Borramos la informacion del cliente que sale
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. Desconexión procesada\n");
                                                send(i, buffer, strlen(buffer), 0);

                                                salirCliente(i,&readfds,&numClientes,arrayClientes);

                                                // Limpiamos el tablero
                                                partida[pos].emptyBoard();
                                            }

                                            else
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "-ERR. Turno rival. Esperando turno...\n");
                                                send(i, buffer, strlen(buffer), 0);
                                            }

                                        break;

                                        // Turno de colocar
                                        case 4:                 // ACABADO
                                             //COLOCAR FICHA
                                            if(strstr(buffer,"COLOCAR-FICHA ") != NULL)
                                            {

                                                //Sacamos "COLOCAR-FICHA" del buffer
                                                aux = strtok(buffer, " ,");

                                                // Sacamos la ficha y la posicion donde colocarla (izquierda/derecha)
                                                aux = strtok(NULL, " ,");   //La ficha entra en aux
                                                contNum = 0;                //Evaluamos si es un comando de colocar ficha válido

                                                //Hallamos los valores de la ficha
                                                for (int l = 0; l < strlen(aux); l++) {
                                                    if (isdigit(aux[l])) {
                                                        if(contNum < 2){
                                                            // "- '0'" sirve para pasar de char a int el contenido de aux
                                                            valorFicha[contNum] = (aux[l]) - '0';
                                                        }
														contNum++;
                                                    }
                                                }
												if(contNum != 2){   //Si el comando no lleva exactamente 2 valores numéricos
													bzero(buffer, sizeof(buffer));
													strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Ficha inválida.\n");
													send(i, buffer, strlen(buffer), 0);
													break;
												}

                                                extremo = strtok(NULL, " ,");   //El extremo entra en aux

                                                //Si el valor del extremo no es válido
                                                if ((strcmp( extremo, "derecha\n") != 0 ) && (strcmp( extremo, "izquierda\n") != 0 ))
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Extremo inválido.\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                    break;
                                                }


                                                auxFicha.left = valorFicha[0];
                                                auxFicha.right = valorFicha[1];

												// Obtenemos la partida en la que jugaba el cliente
												pos = arrayClientes[clienteX].inGame;

                                                //Se coloca la ficha y se borra de la mano del jugador
                                                //partida[pos].hasFicha(auxFicha, partida[pos].getHand1())
                                                if(i == partida[pos].getSocketP1()){    //Se comprueba si P1, ha jugado la ficha
                                                    if (partida[pos].hasFicha(auxFicha, partida[pos].getHand1()))    //Se comprueba que tenga dicha ficha
                                                    {
														if (strcmp( extremo, "izquierda\n") == 0)
														{
															//Si la ficha se puede coloca, se coloca y se le quita de la mano
															if (partida[pos].putInBoardLeft(auxFicha)) {
																partida[pos].quitPieceJ1(auxFicha);
															}
															else{
																bzero(buffer, sizeof(buffer));
																strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Imposible colocar la ficha en ese extremo.\n");
																send(i, buffer, strlen(buffer), 0);
																break;
															}
														}

														else if (strcmp( extremo, "derecha\n") == 0)
														{
															//Si la ficha se puede coloca, se coloca y se le quita de la mano
															if (partida[pos].putInBoardRight(auxFicha)) {
																partida[pos].quitPieceJ1(auxFicha);
															}
															else{
																bzero(buffer, sizeof(buffer));
																strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Imposible colocar la ficha en ese extremo.\n");
																send(i, buffer, strlen(buffer), 0);
																break;
															}
														}

														else
														{
															printf("-ERR. Caso imposible 1, esto no deberia ocurrir. Extremo no reconocido.\n");
														}
                                                    }

                                                    else{
                                                        bzero(buffer, sizeof(buffer));
                                                        strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Ficha no disponible.\n");
                                                        send(i, buffer, strlen(buffer), 0);
                                                        break;
                                                    }
                                                }
                                                else if(i == partida[pos].getSocketP2()){   //O si ha sido P2
                                                    if (partida[pos].hasFicha(auxFicha, partida[pos].getHand2()))    //Se comprueba que tenga dicha ficha
                                                    {
														if (strcmp( extremo, "izquierda\n") == 0)
														{
															//Si la ficha se puede coloca, se coloca y se le quita de la mano
															if (partida[pos].putInBoardLeft(auxFicha)) {
																partida[pos].quitPieceJ2(auxFicha);
															}
															else{
																bzero(buffer, sizeof(buffer));
																strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Imposible colocar la ficha en ese extremo.\n");
																send(i, buffer, strlen(buffer), 0);
																break;
															}
														}

														else if (strcmp( extremo, "derecha\n") == 0)
														{
															//Si la ficha se puede coloca, se coloca y se le quita de la mano
															if (partida[pos].putInBoardRight(auxFicha)) {
																partida[pos].quitPieceJ2(auxFicha);
															}
															else{
																bzero(buffer, sizeof(buffer));
																strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Imposible colocar la ficha en ese extremo.\n");
																send(i, buffer, strlen(buffer), 0);
																break;
															}
														}

														else
														{
															printf("-ERR. Caso imposible 2, esto no deberia ocurrir. Extremo no reconocido.\n");
														}
                                                    }
                                                    else{
                                                        bzero(buffer, sizeof(buffer));
                                                        strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Ficha no disponible.\n");
                                                        send(i, buffer, strlen(buffer), 0);
                                                        break;
                                                    }
                                                }
                                                else{
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Comando colocación de ficha inválido: Jugador no reconocido..\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                    break;
                                                }

												//Se comprueba si era su ultima ficha y ha ganado
												if(i == partida[pos].getSocketP1()){
													if(partida[pos].isHand1Empty()){
														strcpy(mensaje, "+Ok. Partida Finalizada. Jugador1 ha ganado la partida.\n\n");
														send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
														send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);
														//Se informa al servidor también
														printf("+Ok. Partida finalizada\n");
														exit(1);
													}
												}
												else{
													if(partida[pos].isHand2Empty()){
														strcpy(mensaje, "+Ok. Partida Finalizada. Jugador2 ha ganado la partida.\n\n");
														send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
														send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);
														//Se informa al servidor también
														printf("+Ok. Partida finalizada\n");
														exit(2);
													}
												}

												//Se informa de paso de turno
												if(i == partida[pos].getSocketP1()){
													strcpy(mensaje, "+Ok. Turno de partida.\n\n");
													send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);
												}
												else if (i == partida[pos].getSocketP2()) {
													strcpy(mensaje, "+Ok. Turno de partida.\n\n");
													send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
												}
												else{
													printf("-ERR. Comando PASO-TURNO inválido: Jugador no reconocido.");
												}

                                                // Enviamos el tablero
                                                strcpy(mensaje, partida[pos].showBoard().c_str());
                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                // Send hands
                                                strcpy(mensaje, partida[pos].messageHandP1().c_str());
                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);

                                                strcpy(mensaje, partida[pos].messageHandP2().c_str());
                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                                //Cambiamos los estados, correspondientes al cambio de turno
                                                alternatePlayerStatus(partida[pos], arrayClientes);
                                            }



                                            //ROBAR FICHA
                                            else if (strstr(buffer,"ROBAR-FICHA\n") != NULL)
                                            {

												// Obtenemos la partida en la que jugaba el cliente
												pos = arrayClientes[clienteX].inGame;

												if(canPutPiece(i, partida[pos])){
													strcpy(mensaje, "+Ok. No es necesario robar ficha.\n\n");
	                                                send(i, mensaje, strlen(mensaje), 0);
													break;
												}
												else{
													if(partida[pos].isForStoleEmpty()){
														strcpy(mensaje, "-ERR. No quedan fichas para robar.\n\n");
		                                                send(i, mensaje, strlen(mensaje), 0);
														break;
													}
													else{
														auxFicha = partida[pos].stealPiece();
														partida[pos].addToHand(i, auxFicha);
													}
												}

												//Se informa de la ficha robada
												auxstr = "FICHA\t";
												auxstr = auxstr + "|" + std::to_string(auxFicha.left) + "|" + std::to_string(auxFicha.right) + "|\n\n";
												strcpy(mensaje, auxstr.c_str());
												send(i, mensaje, strlen(mensaje), 0);

												//Se informa del estado de la partida
												strcpy(mensaje, "+Ok. Ficha robada. Continúa el mismo jugador.\n\n");
												send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
												send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

												// Enviamos el tablero
												strcpy(mensaje, partida[pos].showBoard().c_str());
												send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
												send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

												// Send hands
												strcpy(mensaje, partida[pos].messageHandP1().c_str());
												send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);

												strcpy(mensaje, partida[pos].messageHandP2().c_str());
												send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

                                            }



                                            //PASO TURNO
                                            else if (strstr(buffer,"PASO-TURNO\n") != NULL)
                                            {
                                                // Obtenemos la partida en la que jugaba el cliente
												pos = arrayClientes[clienteX].inGame;

												//Si tienes fichas para colocar no puedes pasar turno
												if(canPutPiece(i, partida[pos])){
													strcpy(mensaje, "+Ok. No es necesario pasar turno.\n\n");
													send(i, mensaje, strlen(mensaje), 0);
													break;
												}
												//Si aun quedan fichas para robar no se puede pasar turno
												else if (!(partida[pos].isForStoleEmpty())) {
													strcpy(mensaje, "+Ok. No es necesario pasar turno.\n\n");
													send(i, mensaje, strlen(mensaje), 0);
													break;
												}
												//Si el otro jugador tampoco puede poner la partida esta cerrada
												else if (i == partida[pos].getSocketP1()) {
													if (!(canPutPiece(partida[pos].getSocketP2(), partida[pos]))) {
														partida[pos].partidaCerrada();
													}
												}
												else if (i == partida[pos].getSocketP2()) {
													if (!(canPutPiece(partida[pos].getSocketP1(), partida[pos]))) {
														partida[pos].partidaCerrada();
													}
												}
												else{
	                                                //Se informa de paso de turno
													if(i == partida[pos].getSocketP1()){
														strcpy(mensaje, "+Ok. Turno de partida.\n\n");
		                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);
													}
													else if (i == partida[pos].getSocketP2()) {
														strcpy(mensaje, "+Ok. Turno de partida.\n\n");
		                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
													}
													else{
														printf("-ERR. Comando PASO-TURNO inválido: Jugador no reconocido.");
													}

	                                                // Enviamos el tablero
	                                                strcpy(mensaje, partida[pos].showBoard().c_str());
	                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);
	                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);

	                                                // Send hands
	                                                strcpy(mensaje, partida[pos].messageHandP1().c_str());
	                                                send(partida[pos].getSocketP1(), mensaje, strlen(mensaje), 0);

	                                                strcpy(mensaje, partida[pos].messageHandP2().c_str());
	                                                send(partida[pos].getSocketP2(), mensaje, strlen(mensaje), 0);



	                                                //Cambiamos los estados, correspondientes al cambio de turno
	                                                alternatePlayerStatus(partida[pos], arrayClientes);
												}
                                            }

                                            else if(strstr(buffer, "SALIR\n") != NULL)
                                            {
                                                // Obtenemos la partida en la que jugaba el cliente
                                                pos = arrayClientes[clienteX].inGame;

                                                // Mandamos el mensaje de partida anulada a los jugadores
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. La partida ha sido anulada.\n\n");
                                                send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);
                                                send(partida[pos].getSocketP2(), buffer, strlen(buffer), 0);

                                                // Comprobamos cual es el jugador que NO sale
                                                if(arrayClientes[clienteX].sd == partida[pos].getSocketP1())
                                                {
                                                    for( j = 0; j < numClientes; j++)
                                                    {
                                                        if(arrayClientes[j].sd == partida[pos].getSocketP2())
                                                        {
                                                            // Cambiamos el estado para que pueda iniciar otra partida
                                                            arrayClientes[j].status = 2;
                                                        }
                                                    }
                                                }

                                                else if(arrayClientes[clienteX].sd == partida[pos].getSocketP2())
                                                {
                                                    for( j = 0; j < numClientes; j++)
                                                    {
                                                        if(arrayClientes[j].sd == partida[pos].getSocketP1())
                                                        {
                                                            // Cambiamos el estado para que pueda iniciar otra partida
                                                            arrayClientes[j].status = 2;
                                                        }
                                                    }
                                                }

                                                else
                                                {
                                                    std::cout << "Esto no deberia de pasar....\n";
                                                }

                                                // Borramos la informacion del cliente que sale
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "+Ok. Desconexión procesada\n");
                                                send(i, buffer, strlen(buffer), 0);

                                                salirCliente(i,&readfds,&numClientes,arrayClientes);

                                                // Limpiamos el tablero
                                                partida[pos].emptyBoard();
                                            }

                                            //COMANDO NO RECONOCIDO -- Se le recuerda al jugador qué puede hacer
                                            else{
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "-ERR. Comando inválido. (Comandos válidos: COLOCAR-FICHA <|valor·valor|>,<extremo>; ROBAR-FICHA; PASO-TURNO). Es tu turno...\n");
                                                send(i, buffer, strlen(buffer), 0);
                                            }

                                        break;

                                    }




                            }
                            //Si el cliente introdujo ctrl+c
                            if(recibidos== 0)
                            {
                                printf("+Ok. El socket %d, ha introducido ctrl+c\n", i);
                                //Eliminar ese socket
                                salirCliente(i,&readfds,&numClientes,arrayClientes);
                            }
                        }
                    }
                }
            }
		}

	close(sd);
	return 0;

}

void salirCliente(int socket, fd_set * readfds, int * numClientes, std::vector<cliente> arrayClientes){

    char buffer[250];
    int j;

    close(socket);
    FD_CLR(socket,readfds);

    //Re-estructurar el array de clientes
    for (j = 0; j < (*numClientes) - 1; j++)
        if (arrayClientes[j].sd == socket)
            break;
    for (; j < (*numClientes) - 1; j++)
        (arrayClientes[j] = arrayClientes[j+1]);

    (*numClientes)--;

    bzero(buffer,sizeof(buffer));
    sprintf(buffer,"+Ok. Desconexión del cliente: %d\n",socket);

    for(j=0; j<(*numClientes); j++)
        if(arrayClientes[j].sd != socket)
            send(arrayClientes[j].sd,buffer,strlen(buffer),0);


}


void manejador (int signum){
    printf("\n+Ok. Se ha recibido la señal sigint\n");
    signal(SIGINT,manejador);

    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}
