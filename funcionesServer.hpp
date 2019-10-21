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