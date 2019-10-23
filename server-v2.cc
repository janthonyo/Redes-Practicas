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
#define MAX_CLIENTS 50
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

    int numClientes = 0;
    int numPartidas = 0;
    //contadores
    int i,j,k;
	int recibidos;
    char identificador[MSG_SIZE];

    // Cadenas auxiliares
    char *aux;  
    char *user;
    char *passwd;
    char mensaje[MSG_SIZE];
    int pos, result;
    
    int on, ret;

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
    		exit (1);	
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
		exit(1);
	}
	

   	/*---------------------------------------------------------------------
		Del las peticiones que vamos a aceptar sólo necesitamos el 
		tamaño de su estructura, el resto de información (familia, puerto, 
		ip), nos la proporcionará el método que recibe las peticiones.
   	----------------------------------------------------------------------*/
		from_len = sizeof (from);


		if(listen(sd,1) == -1){
			perror("Error en la operación de listen");
			exit(1);
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
                                    numClientes++;
                                    FD_SET(new_sd,&readfds);
                                
                                    bzero(buffer,sizeof(buffer));
                                    strcpy(buffer, "+0k. Usuario conectado.\n");
                                    send(new_sd,buffer,strlen(buffer),0);
                                }
                                else
                                {
                                    bzero(buffer,sizeof(buffer));
                                    strcpy(buffer,"Demasiados clientes conectados\n");
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
                                    send(arrayClientes[j].sd, "Desconexion servidor\n", strlen("Desconexion servidor\n"),0);
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
                                
                                if(strcmp(buffer,"SALIR\n") == 0){
                                    
                                    salirCliente(i,&readfds,&numClientes,arrayClientes);
                                    
                                }
                                else{

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
                                            if( strstr(buffer, "USUARIO" ) != NULL ) 
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

                                            else if ( strstr(buffer, "REGISTRO" ) != NULL )
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
                                            if( strstr(buffer, "PASSWORD" ) != NULL )
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

                                                    //Actualizamos su estado
                                                    arrayClientes[clienteX].status = 2;
                                                }

                                                else
                                                {
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-ERR. Error en la validacion\n");
                                                    send(i, buffer, strlen(buffer), 0);
                                                }
                                            }

                                            else
                                            {
                                                // Mandamos un mensaje al usuario para recordarle que hace en este
                                                // estado.

                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "> Introduzca la contraseña.\n");
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
                                                                //Enviamos que jugador ha sacado.
                                                                strcpy(mensaje, "+Jugador 2 (");
                                                                strcat(mensaje, partida[pos].getUserP2());
                                                                strcat(mensaje, ") sale y ha colocado ficha.\n\n");

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
                                                                //Enviamos que jugador ha sacado.
                                                                strcpy(mensaje, "+Jugador 1 (");
                                                                strcat(mensaje, partida[pos].getUserP1());
                                                                strcat(mensaje, ") sale y ha colocado ficha.\n\n");

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
                                        case 3:                 // POR HACER
                                            bzero(buffer, sizeof(buffer));
                                            strcpy(buffer, "Estado 3. Esperando turno...\n");
                                            send(i, buffer, strlen(buffer), 0);
                                        break;

                                        // Turno de colocar
                                        case 4:                 // POR HACER
                                            bzero(buffer, sizeof(buffer));
                                            strcpy(buffer, "Estado 4. Toca colocar...\n");
                                            send(i, buffer, strlen(buffer), 0);
                                        break;

                                    }
                                    
                                }
                                                                
                                
                            }
                            //Si el cliente introdujo ctrl+c
                            if(recibidos== 0)
                            {
                                printf("El socket %d, ha introducido ctrl+c\n", i);
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
    sprintf(buffer,"Desconexión del cliente: %d\n",socket);
    
    for(j=0; j<(*numClientes); j++)
        if(arrayClientes[j].sd != socket)
            send(arrayClientes[j].sd,buffer,strlen(buffer),0);


}


void manejador (int signum){
    printf("\nSe ha recibido la señal sigint\n");
    signal(SIGINT,manejador);
    
    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}
