#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

//Includes añadidos
#include <ctype.h>

int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd;
	struct sockaddr_in sockname;
	char buffer[250];
	socklen_t len_sockname;
    fd_set readfds, auxfds;
    int salida;
    int fin = 0;

    //Variables añadidas
    int numbers[100];
    int numbers_len = 0;


	/* --------------------------------------------------
		Se abre el socket
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (1);
	}



	/* ------------------------------------------------------------------
		Se rellenan los campos de la estructura con la IP del
		servidor y el puerto del servicio que solicitamos
	-------------------------------------------------------------------*/
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2050);
	sockname.sin_addr.s_addr =  inet_addr("192.168.1.131");

	/* ------------------------------------------------------------------
		Se solicita la conexión con el servidor
	-------------------------------------------------------------------*/
	len_sockname = sizeof(sockname);

	if (connect(sd, (struct sockaddr *)&sockname, len_sockname) == -1)
	{
		perror ("Error de conexión");
		exit(1);
	}

    //Inicializamos las estructuras
    FD_ZERO(&auxfds);
    FD_ZERO(&readfds);

    FD_SET(0,&readfds);
    FD_SET(sd,&readfds);


	/* ------------------------------------------------------------------
		Se transmite la información (Parte a modificar)
	-------------------------------------------------------------------*/
	do
	{
        auxfds = readfds;
        salida = select(sd+1,&auxfds,NULL,NULL,NULL);

        //Tengo mensaje desde el servidor
        if(FD_ISSET(sd, &auxfds)){

            bzero(buffer,sizeof(buffer));
            recv(sd,buffer,sizeof(buffer),0);

            //printf("\n%s\n",buffer);

            if(strcmp(buffer,"Demasiados clientes conectados\n") == 0)
                fin =1;

            if(strcmp(buffer,"Desconexion servidor\n") == 0)
                fin =1;

            //Formatea de forma bonita las fichas
            numbers_len = 0;
            for (int i = 0; i < strlen(buffer); i++) {
                if (buffer[i]=='|') {
                    if( isdigit(buffer[i-1]) && isdigit(buffer[i+1]) ){
                        printf("·");
                    }
                    else{
                        printf("%c", buffer[i]);
                    }
                }
                else{
                    printf("%c", buffer[i]);
                }
            }

            for (int i = 0; i < numbers_len; i++) {
                printf("|%d·", numbers[i]);
                i++;
                printf("%d|\n", numbers[i]);
            }


        }
        else
        {

            //He introducido información por teclado
            /*(No hay que cambiarlo. El cliente escribe el numero de la opción
            que quiere escoger, este se manda al servidor y él lo interpreta.)*/
            if(FD_ISSET(0,&auxfds)){
                bzero(buffer,sizeof(buffer));

                fgets(buffer,sizeof(buffer),stdin);

                if(strcmp(buffer,"SALIR\n") == 0){
                        fin = 1;

                }

                send(sd,buffer,sizeof(buffer),0);

            }


        }



    }while(fin == 0);

    close(sd);

    return 0;

}
