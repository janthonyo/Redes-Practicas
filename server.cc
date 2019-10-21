#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

#include <iostream>

#define MSG_SIZE 250
#define MAX_CLIENTS 30


void manejador(int signum);
void salirCliente(int socket, fd_set * readfds, int * numClientes, int arrayClientes[]);


int main()
{
	/*---------------------------------------------------- 
		Descriptor del socket y buffer de datos                
	-----------------------------------------------------*/
	int sd, new_sd;
	struct sockaddr_in sockname, from;
	char buffer[100];
	socklen_t from_len;
	fd_set readfds, auxfds;
    int salida;
    int arrayClientes[MAX_CLIENTS];
    int numClientes = 0;
    //contadores
    int i,j,k;
	int recibidos;
    char identificador[MSG_SIZE];
    
	int on, ret;
    char * aux;		//String auxiliar para comprobar las acciones del buffer
    int statusAux;


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
	
	int status[MAX_CLIENTS];

	/* --------------------------------------------------
		Se abre el socket 
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (1);	
	}

	on=1;
    ret = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* --------------------------------------------------

	   Se asocia el socket a un puerto con la funcion bind

	   Para ello debemos rellenar una estructura sockaddr_in (en nuestro caso sockname)

	   - AF_INET

	--------------------------------------------------*/

	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2000);
	sockname.sin_addr.s_addr =  INADDR_ANY;


	if (bind (sd, (struct sockaddr *) &sockname, sizeof (sockname)) == -1)
	{
		perror("Error en la operación bind");
		exit(1);
	}


	from_len = sizeof(from);

	if (listen(sd, 1) == -1)
	{
		perror("ERROR: Fallo en la operacion de listen.");
		exit(1);
	}
	
    //Inicializar los conjuntos fd_set
    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd,&readfds); // Con este FD_SET, recibiremos mensajes de los clientes
    FD_SET(0,&readfds);	 // Con este FD_SET, mandaremos mensajes desde teclado.

    //Capturamos la señal SIGINT (Ctrl+c)
    //signal(SIGINT,manejador);
    
    /*-----------------------------------------------------------------------
		El servidor acepta una petición
	------------------------------------------------------------------------ */
    while(1){

        auxfds = readfds;

        salida = select(FD_SETSIZE, &auxfds, NULL, NULL, NULL);

        if(salida > 0){

        	for(i=0; i<FD_SETSIZE; i++)
        	{
        		//Buscamos el socket por el que se ha establecido la comunicacion
        		if(FD_ISSET(i, &auxfds))
        		{

        			if( i == sd ) //Peticion de un nuevo cliente
        			{
        				if((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
                            perror("Error aceptando peticiones");
                        }
                        else
                        {
                            if(numClientes < MAX_CLIENTS)
                            {
                            	//Guardamos al nuevo cliente 
                            	arrayClientes[numClientes] = new_sd;
                            	status[numClientes] = 0;
                            	numClientes++;
                            	FD_SET(new_sd, &readfds);

                            	//Pedimos que se loguee / registre
                            	bzero(buffer,sizeof(buffer));
                            	strcpy(buffer, "+0k. Usuario conectado\n\n");
                            	send(new_sd, buffer, strlen(buffer), 0);
                            }

                            //Maximo de clientes conectados
                            else
							{
                                bzero(buffer,sizeof(buffer));
                                strcpy(buffer,"Demasiados clientes conectados\n");
                                send(new_sd,buffer,strlen(buffer),0);
                                close(new_sd);
                            }
                            
                        }
                    }

                    else if (i == 0)
                	{
                        //Se ha introducido información de teclado
                        bzero(buffer, sizeof(buffer));
                        fgets(buffer, sizeof(buffer),stdin);
                        
                        //Controlar si se ha introducido "SALIR", cerrando todos los sockets y finalmente saliendo del servidor. (implementar)
                        if(strcmp(buffer,"SALIR\n") == 0){
                         
                            for (j = 0; j < numClientes; j++){
                                send(arrayClientes[j], "Desconexion servidor\n", strlen("Desconexion servidor\n"),0);
                                close(arrayClientes[j]);
                                FD_CLR(arrayClientes[j],&readfds);
                            }
                                close(sd);
                                exit(-1);
                        }
                        //Mensajes que se quieran mandar a los clientes (implementar)  
                    }

                    else
                    {
                        bzero(buffer,sizeof(buffer));
                        //std::cout << "Socket: " << std::endl;
                        
                        // Comprobamos el socket de i por donde se enviaran los datos.
                        recibidos = recv(i,buffer,sizeof(buffer),0);
                        
                        // Si ha escrito:
                        if(recibidos > 0)
                        {
                            
                            if(strcmp(buffer,"SALIR\n") == 0)
                            {
                                //Comprobamos si quiere salir
                                salirCliente(i,&readfds,&numClientes,arrayClientes);
                            }

                            else
                            {
                                for(j=0; j<numClientes; j++)
                                {
                                    //std::cout << j << std::endl;

                                    if(arrayClientes[j] == i)
                                    {
                                    	std::cout << i << std::endl;
                                    	statusAux = status[j];
                                    }
                                }

                                switch(statusAux)
                            	{
                            		case 0: //Esperando usuario o registro

                            			// El cliente introduce el usuario para loguearse
                            			if( strstr(buffer, "USUARIO" ) != NULL ) 
                            			{

                							std::cout << "Login" << std::endl;

                            				aux = strtok(buffer, " ");

                            				aux = strtok(NULL, "\n");
                            			
                            				if (aux != NULL)	std::cout << aux << std::endl;

                            				// Una vez comprobado si el usuario esta o no,
                            				// mandamos el mensaje y cambiamos el estado si es
                            				// correcto

                            				bzero(buffer, sizeof(buffer));
                            				strcpy(buffer, "+Ok. Usuario correcto");
                            				send(i, buffer, strlen(buffer), 0);
                            			}

                            			// El cliente se registra en el domino
                            			else if ( strstr(buffer, "REGISTRO" ) != NULL )
                            			{
                							std::cout << "Login (registro)" << std::endl;

                            				aux = strtok(buffer, " ");

                            				while( aux != NULL)
                            				{
                            					if (strcmp(aux, "-u" ) == 0 )
                                				{
                                					aux = strtok(NULL, " ");

                                					if(aux != NULL) std::cout << "1: " << aux << "\n";
                                				}

                                				if (strcmp(aux, "-p") == 0)
                                				{
                                					aux = strtok(NULL, "\n");

                                					if (aux != NULL) std::cout << "2: " << aux << "\n";
                                				}

                                				aux = strtok(NULL, " ");

                            				}

                            				bzero(buffer, sizeof(buffer));
                            				strcpy(buffer, "+Ok. Usuario registrado");
                            				send(i, buffer, strlen(buffer), 0);
                            			}

                            		break;

                            		case 1:
                						std::cout << "> Contraseña" << std::endl;

                            			bzero(buffer, sizeof(buffer));
                            			strcpy(buffer, "(En process...) Parte de la contraseña.\n");
                            			send(i, buffer, strlen(buffer), 0);
                            		break;
                            	} // Switch
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


void salirCliente(int socket, fd_set * readfds, int * numClientes, int arrayClientes[]){
  
    char buffer[250];
    int j;

    //Se cierra el socket del cliente y se elimina del conjunto en readfds
    close(socket);
    FD_CLR(socket,readfds);
    
    //Re-estructurar el array de clientes
    for (j = 0; j < (*numClientes) - 1; j++)
        if (arrayClientes[j] == socket)
            break;
    for (; j < (*numClientes) - 1; j++)
        (arrayClientes[j] = arrayClientes[j+1]);
    
    (*numClientes)--;
    
    bzero(buffer,sizeof(buffer));
    sprintf(buffer,"Desconexión del cliente: %d\n",socket);
    
    for(j=0; j<(*numClientes); j++)
        if(arrayClientes[j] != socket)
            send(arrayClientes[j],buffer,strlen(buffer),0);


}


void manejador (int signum){
    printf("\nSe ha recibido la señal sigint\n");
    //signal(SIGINT,manejador);
    
    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}