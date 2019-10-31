#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <unistd.h>
#include <signal.h>

#include "funcionesServer.hpp"

void salirCliente(int socket, fd_set * readfds, int * numClientes, std::vector<cliente> &arrayClientes){

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
}


void manejador (int signum){
    printf("\n+Ok. Se ha recibido la señal sigint\n");
    signal(SIGINT,manejador);

    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}

bool checkUser(char buffer[])
{
	char user[100];
	char passwd[100];
	std::fstream dataFile;
	bool isUser = false;

	dataFile.open("Data-Clients.txt", std::fstream::in);

	// A la fuerza, cualquier cliente tendra que tener
	// un usuario y una contraseña para acceder al domino y,
	// por lo tanto en el fichero de datos deben aparecer ambos reflejados,
	// por lo que recorreremos el fichero guardando el valor
	// del nombre de usuario (user) y su password (passwd)

	while(!dataFile.eof())
	{
		dataFile >> user;
		dataFile >> passwd;

		if( strcmp(buffer, user) == 0 )
			isUser = true;
	}

	dataFile.close();

	return isUser;
}

bool registerUser(char rUser[], char rPasswd[])
{
	bool registerCompleted = false;
	std::fstream dataFile;

	if(!checkUser(rUser))
	{
		dataFile.open("Data-Clients.txt", std::fstream::out | std::fstream::app);
		dataFile << rUser << " " << rPasswd << "\n";
		dataFile.close();
		registerCompleted = true;
	}

	return registerCompleted;
}

bool verifyPassword(char cUser[], char cPasswd[])
{
	char user[100];
	char passwd[100];
	bool foundClient = false;
	bool correctPasswd = false;

	std::fstream dataFile;
	dataFile.open("Data-Clients.txt", std::fstream::in);

	while ((!dataFile.eof()) and (!foundClient))
	{
		dataFile >> user;
		dataFile >> passwd;

		if(strcmp(cUser, user) == 0)
		{
			foundClient = true;

			if(strcmp(cPasswd, passwd) == 0)
			{
				correctPasswd = true;
			}
		}
	}

	dataFile.close();

	return correctPasswd;
}

int matchmaking(std::vector<domino> games)
{
	int gameAssigned = -1;

	for(int i = 0; i < (int) games.size(); i++)
	{
		if(gameAssigned == -1)
		{
			if(!games[i].has2players())
			{
				gameAssigned = i;
			}
		}
	}

	return gameAssigned;
}


void alternatePlayerStatus(domino& game, std::vector<cliente>& arrayClientes)
{
	int sdplayer1 = game.getSocketP1();
	int sdplayer2 = game.getSocketP2();

	for (int i = 0; i < (int) arrayClientes.size(); i++)
	{
		if (arrayClientes[i].sd == sdplayer1)
        {
        	if (arrayClientes[i].status == 3)
        		arrayClientes[i].status = 4;

        	else
        		arrayClientes[i].status = 3;
        }

        else if (arrayClientes[i].sd == sdplayer2)
        {
        	if (arrayClientes[i].status == 3)
        		arrayClientes[i].status = 4;

        	else
        		arrayClientes[i].status = 3;
        }
	}

}

bool canPutPiece(int i, domino& game)
{
    //Devuelve true si tienes piezas disponibles para colocar, false si no
    //i es el sd de quien envio el mensaje, tambien identificado como i en server.cc
    if(i==game.getSocketP1())
    {
        for (int j = 0; j < (int) game.getHand1().size(); j++) {
            //Comprueba si coincide first
            if(game.getFirst() == game.getHand1()[j].left)
            {
                return true;
            }
            if(game.getFirst() == game.getHand1()[j].right)
            {
                return true;
            }

            //Comprueba si coincide last
            if(game.getLast() == game.getHand1()[j].left)
            {
                return true;
            }
            if(game.getLast() == game.getHand1()[j].right)
            {
                return true;
            }
        }
    }

    else if(i==game.getSocketP2())
    {
        for (int j = 0; j < (int) game.getHand2().size(); j++) {
            //Comprueba si coincide first
            if(game.getFirst() == game.getHand2()[j].left)
            {
                return true;
            }
            if(game.getFirst() == game.getHand2()[j].right)
            {
                return true;
            }

            //Comprueba si coincide last
            if(game.getLast() == game.getHand2()[j].left)
            {
                return true;
            }
            if(game.getLast() == game.getHand2()[j].right)
            {
                return true;
            }
        }
    }

    return false;
}


bool userInWaitList(int sd, std::vector <int> waitList)
{
	bool result = false;

	for(int i = 0; i < (int) waitList.size(); i++)
	{
		if(waitList[i] == sd)
			result = true;

	}

	return result;
}

void managePostGame(std::vector<domino> &partida, int pos, std::vector<cliente> &arrayClientes, int numClientes, int *numPartidas)
{
	for (int j = 0; j < numClientes; j++)
	{
	    if (arrayClientes[j].sd == partida[pos].getSocketP1())
	    {
	        // Cambiamos el estado para que puedan iniciar nueva partida
	        arrayClientes[j].status = 2;
	        arrayClientes[j].inGame = -1;
	    }

	    else if (arrayClientes[j].sd == partida[pos].getSocketP2())
	    {
	        // Cambiamos el estado para que puedan iniciar nueva partida
	        arrayClientes[j].status = 2;
	        arrayClientes[j].inGame = -1;
	    }
    }

    // Limpiamos la partida y la habilitamos para otros usuarios
    partida[pos].emptyBoard();
    (*numPartidas)--;
}

void waitListGame(std::vector <int> &lista_espera, std::vector<domino> &partida, int pos, std::vector<cliente> &arrayClientes, int numClientes, int *numPartidas)
{
	int size_lista = (int) lista_espera.size();
	int result;
	char buffer[MSG_SIZE];
	char mensaje[MSG_SIZE];


	// Si solo hay un jugador en la lista
	if (size_lista == 1)
	{

		for (int i = 0; i < numClientes; i++)
		{

			if (arrayClientes[i].sd == lista_espera[0])
			{

				// Establecemos al usuario en partida
				partida[pos].setPlayer1(arrayClientes[i].user);
				partida[pos].setSocket1(arrayClientes[i].sd);

				// Guardamos en el cliente la partida asignada
				arrayClientes[i].inGame = pos;

				// Mandamos un mensaje de que hemos encontrado una partida disponible
				bzero(buffer, sizeof(buffer));
				strcpy(buffer, "+Ok. Partida disponible. Quedamos a la espera de mas jugadores\n");
				send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);

				// Borramos al usuario de la lista de espera
				lista_espera.erase(lista_espera.begin());
			}
		}
	}

	// Mas de un jugador en la lista
	else if (size_lista >= 2)
	{

		// Buscamos los clientes en el arrayClientes para asignar los datos
		for (int i = 0; i< numClientes; i++)
		{
			if (arrayClientes[i].sd == lista_espera[0])
			{
				// Establecemos al usuario como J1 en partida
				partida[pos].setPlayer1(arrayClientes[i].user);
				partida[pos].setSocket1(arrayClientes[i].sd);

				// Guardamos en el cliente la partida asignada
				arrayClientes[i].inGame = pos;
			}

			else if (arrayClientes[i].sd == lista_espera[1])
			{
				// Establecemos al usuario como J2 en partida
				partida[pos].setPlayer2(arrayClientes[i].user);
				partida[pos].setSocket2(arrayClientes[i].sd);

				// Guardamos en el cliente la partida asignada
				arrayClientes[i].inGame = pos;
			}
		}

		// Borramos a los usuarios de la lista de espera
		lista_espera.erase(lista_espera.begin()); 			// Borramos al 1er usuario
		lista_espera.erase(lista_espera.begin());			// Borramos al 2o. usuario (el nuevo primero)

		// Mandamos un mensaje a los jugadores de que empieza la partida.
		bzero(buffer, sizeof(buffer));
		strcpy(buffer, "+Ok. Empieza la partida");
		send(partida[pos].getSocketP1(), buffer, strlen(buffer), 0);
		send(partida[pos].getSocketP2(), buffer, strlen(buffer), 0);

		(*numPartidas)++;

		// Establecemos las manos
		partida[pos].setHand1(partida[pos].newHand());
		partida[pos].setHand2(partida[pos].newHand());

		// Vemos quien sale
		result = partida[pos].startPlayer();

		startGameMessage(partida[pos], result, &numClientes, arrayClientes);
	}

	// Si la lista esta vacia o ya se han hecho las operaciones, no se hace nada mas
}

bool checkPiece(int i, char* buffer, ficha &auxFicha, char* &extremo)
{
    //Variables auxiliares
    int contNum = 0;
    char valorFicha[2];
    char* aux;

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
        return false;
    }

    extremo = strtok(NULL, " ,");   //El extremo entra en aux

    //Si el valor del extremo no es válido
    if ((strcmp( extremo, "derecha\n") != 0 ) && (strcmp( extremo, "izquierda\n") != 0 ))
    {
        return false;
    }

    auxFicha.left = valorFicha[0];
    auxFicha.right = valorFicha[1];

    return true;
}

bool puttingpieceP1(domino& game, ficha &auxFicha, char* extremo)
{
    if (strcmp( extremo, "izquierda\n") == 0)
    {
        //Si la ficha se puede coloca, se coloca y se le quita de la mano
        if (game.putInBoardLeft(auxFicha)) {
            game.quitPieceJ1(auxFicha);
        }
        else{
            return false;
        }
    }

    else if (strcmp( extremo, "derecha\n") == 0)
    {
        //Si la ficha se puede coloca, se coloca y se le quita de la mano
        if (game.putInBoardRight(auxFicha)) {
            game.quitPieceJ1(auxFicha);
        }
        else{
            return false;
        }
    }

    return true;
}

bool puttingpieceP2(domino& game, ficha &auxFicha, char* extremo)
{
    if (strcmp( extremo, "izquierda\n") == 0)
    {
        //Si la ficha se puede coloca, se coloca y se le quita de la mano
        if (game.putInBoardLeft(auxFicha)) {
            game.quitPieceJ2(auxFicha);
        }
        else{
            return false;
        }
    }

    else if (strcmp( extremo, "derecha\n") == 0)
    {
        //Si la ficha se puede coloca, se coloca y se le quita de la mano
        if (game.putInBoardRight(auxFicha)) {
            game.quitPieceJ2(auxFicha);
        }
        else{
            return false;
        }
    }
    return true;
}
