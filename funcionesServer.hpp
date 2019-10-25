#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

#include "domino.cc"

struct cliente
{
    int sd;
    int status;
    char user[100];
    char passwd[100];
    int inGame;	 // Indica la partida en la que esta jugando.
};

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

		if( strstr(buffer, user) != NULL )
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

    else{
        printf("Error - Este caso no deberia ocurrir nunca.\n");
    }

    return false;
}
