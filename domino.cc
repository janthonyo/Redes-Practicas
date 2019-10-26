#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG_SIZE 250

struct ficha
{
	int left;
	int right;
};

struct jugador
{
	char user[100];
	int socket;
	std::vector <ficha> hand;
};

class domino
{
private:
	std::vector<ficha> board;
	std::vector<ficha> forStole; //Monton para robar
	jugador player1;
	jugador player2;
	int first;
	int last;

public:
	domino()
	{
		//Inicializador de numeros aleatorios.
		srand(time(NULL));

		first = -1;
		last = -1;
		board.clear();
		forStole.clear();
		player1.socket = -1;
		player2.socket = -1;

		//"Amontonamos" las fichas para repartir y robar.
		ficha aux;

		for (int i = 0; i < 7; i++)
		{
			for (int j = i; j < 7; j++)
			{
				aux.left = i;
				aux.right = j;

				forStole.push_back(aux);
			}
		}

	}

	//Observadores <--------------------------------------------------

	bool has2players(){return ((player1.socket != -1) and (player2.socket != -1));}

	bool hasPlayer1(){return (player1.socket != -1);}

	bool hasPlayer2(){return (player2.socket != -1);}

	std::vector<ficha> getHand1(){return player1.hand;}

	std::vector<ficha> getHand2(){return player2.hand;}

	char* getUserP1(){return player1.user;}

	char* getUserP2(){return player2.user;}

	int getSocketP1(){return player1.socket;}

	int getSocketP2(){return player2.socket;}

	int getFirst(){return first;}

	int getLast(){return last;}

	int getPiecesForStole(){return (int) forStole.size();}

	bool isForStoleEmpty(){return (forStole.empty());}

	bool isBoardEmpty(){return (board.empty());}

	bool isHand1Empty(){return (player1.hand.size() == 0);}

	bool isHand2Empty(){return (player2.hand.size() == 0);}

	bool hasFicha(ficha f, std::vector<ficha> hand)
	{
		bool result = false;

		for (int i = 0; i < (int) hand.size(); ++i)
		{
			if(( hand[i].left == f.left ) and ( hand[i].right == f.right ))
				result = true;

			else if (( hand[i].left == f.right ) and ( hand[i].right == f.left ))
				result = true;
		}

		return result;
	}

	std::string showBoard()
	{
		std::string tablero("TABLERO\t");

		if (isBoardEmpty())
		{
			tablero = tablero + "VACIO";
		}

		else
		{
			for (int i = 0; i < (int) board.size(); i++)
			{
				tablero = tablero + "|" + std::to_string(board[i].left) + "|" + std::to_string(board[i].right) + "|";
			}
		}

		tablero = tablero +"\n\n";

		return tablero;
	}

	std::string messageHandP1()
	{
		std::string pieces("FICHAS\t");

		for (int i = 0; i < (int) player1.hand.size(); i++)
		{
			pieces = pieces + "|" + std::to_string(player1.hand[i].left) + "|" + std::to_string(player1.hand[i].right) + "|";
		}

		pieces = pieces +"\n\n";

		return pieces;
	}


	std::string messageHandP2()
	{
		std::string pieces("FICHAS\t");

		for (int i = 0; i < (int) player2.hand.size(); i++)
		{
			pieces = pieces + "|" + std::to_string(player2.hand[i].left) + "|" + std::to_string(player2.hand[i].right) + "|";
		}

		pieces = pieces +"\n\n";

		return pieces;
	}

	//Modificadores <--------------------------------------------------

	void setPlayer1(char name[]){strcpy(player1.user, name);}

	void setPlayer2(char name[]){strcpy(player2.user, name);}

	void setSocket1(int sd){player1.socket = sd;}

	void setSocket2(int sd){player2.socket = sd;}

	void setHand1(std::vector<ficha> hand){player1.hand = hand;}

	void setHand2(std::vector<ficha> hand){player2.hand = hand;}

	void emptyBoard()
	{
		board.clear();
		forStole.clear();
		first = -1;
		last = -1;

		//"Amontonamos" las fichas para repartir y robar.
		ficha aux;

		for (int i = 0; i < 7; i++)
		{
			for (int j = i; j < 7; j++)
			{
				aux.left = i;
				aux.right = j;
				forStole.push_back(aux);

				//std::cout << "|" << aux.left << " · " << aux.right << "|\n";
			}
		}
	}

	void setFirst(int x){first = x;}

	void setLast(int x){last = x;}

	std::vector <ficha> newHand()
	{
		std::vector<ficha> playerHand;
		int x;

		for(int i = 0; i < 7; i++)
		{
			x = rand() % (int) forStole.size();

			playerHand.push_back(forStole[x]);

			forStole.erase(forStole.begin() + x);
		}

		return playerHand;
	}


	ficha stealPiece()
	{
		int x = rand() % (int) forStole.size();
		ficha stolenPiece = forStole[x];

		forStole.erase(forStole.begin() + x);

		return stolenPiece;
	}

	bool addToHand(int i, ficha pieceToAdd)
	{
		if(i==getSocketP1())
		{
			player1.hand.push_back(pieceToAdd);
		}
		else if(i==getSocketP2())
		{
			player2.hand.push_back(pieceToAdd);
		}
	}

	bool putInBoard(ficha f)
	{
		//Estructura ficha auxiliar para "girar" la ficha
		ficha aux;
		bool inserted = false;

		if(isBoardEmpty())
		{
			setFirst(f.left);
			setLast(f.right);
			board.push_back(f);

			inserted = true;
		}

		//Se comprueba si la ficha se puede colocar en el extremo inicial.
		else if((first == f.left) or (first == f.right))
		{
			if( f.left == first )
			{
				setFirst(f.right);
				aux.left = f.right;
				aux.right = f.left;
				board.insert(board.begin(), aux);
			}

			else
			{
				setFirst(f.left);
				board.insert(board.begin(), f);
			}

			inserted = true;
		}

		//Si no, se comprueba si la ficha se puede colocar en el extremo final.
		else if((last == f.left) or (last == f.right))
		{
			if( f.left == last )
			{
				setLast(f.right);
				board.push_back(f);
			}

			else
			{
				setLast(f.left);
				aux.left = f.right;
				aux.right = f.left;
				board.push_back(aux);
			}

			inserted = true;
		}

		//Si no, se devuelve false (valor inicial de inserted)
		return inserted;
	}

	bool putInBoardLeft(ficha f)
	{
		//Estructura ficha auxiliar para "girar" la ficha
		ficha aux;
		bool inserted = false;

		/*No hace falta considerar el caso con tablero vacio, ya que nunca se
		  va a llamar a esta funcion en esa situacion.*/

		//Se comprueba si la ficha se puede colocar en el extremo inicial.
		if((first == f.left) or (first == f.right))
		{
			if( f.left == first )
			{
				setFirst(f.right);
				aux.left = f.right;
				aux.right = f.left;
				board.insert(board.begin(), aux);
			}

			else
			{
				setFirst(f.left);
				board.insert(board.begin(), f);
			}

			inserted = true;
		}

		//Si no, se devuelve false (valor inicial de inserted)
		return inserted;
	}

	bool putInBoardRight(ficha f)
	{
		//Estructura ficha auxiliar para "girar" la ficha
		ficha aux;
		bool inserted = false;

		/*No hace falta considerar el caso con tablero vacio, ya que nunca se
		  va a llamar a esta funcion en esa situacion.*/

		  //Se comprueba si la ficha se puede colocar en el extremo final.
  		if((last == f.left) or (last == f.right))
  		{
  			if( f.left == last )
  			{
  				setLast(f.right);
  				board.push_back(f);
  			}

  			else
  			{
  				setLast(f.left);
  				aux.left = f.right;
  				aux.right = f.left;
  				board.push_back(aux);
  			}

  			inserted = true;
  		}

		//Si no, se devuelve false (valor inicial de inserted)
		return inserted;
	}


	int startPlayer()
	{
		ficha aux;

		// Comprobamos quien tiene el doble mayor
		for (int i = 6; i >= 0; i--)
		{
			aux.left = i;
			aux.right = i;

			if(hasFicha(aux, player1.hand))
			{
				if(putInBoard(aux))
				{
					quitPieceJ1(aux);
					return 2;
				}
			}

			else if(hasFicha(aux, player2.hand))
			{
				if(putInBoard(aux))
				{
					quitPieceJ2(aux);
					return 1;
				}
			}

		}

		// Comprobamos quien tiene la ficha mas alta
		for (int i = 6; i >= 0; i--)
		{
			for (int j = i - 1; j >= 0; j--)
			{
				aux.left = i;
				aux.right = j;

				if(hasFicha(aux, player1.hand))
				{
					if(putInBoard(aux))
					{
						quitPieceJ1(aux);
						return 2;
					}
				}

				else if(hasFicha(aux, player2.hand))
				{
					if(putInBoard(aux))
					{
						quitPieceJ2(aux);
						return 1;
					}
				}
			}
		}
	}


	void quitPieceJ1(ficha f)
	{
		bool erase = false;

		for (int i = 0; i < (int) player1.hand.size(); i++)
		{
			if (erase == false)
			{
				if(( player1.hand[i].left == f.left ) and ( player1.hand[i].right == f.right ))
				{
					player1.hand.erase(player1.hand.begin() + i);
					erase = true;
				}

				else if(( player1.hand[i].left == f.right ) and ( player1.hand[i].right == f.left ))
				{
					player1.hand.erase(player1.hand.begin() + i);
					erase = true;
				}
			}
		}
	}


	void quitPieceJ2(ficha f)
	{
		bool erase = false;

		for (int i = 0; i < (int) player2.hand.size(); i++)
		{
			if (erase == false)
			{
				if(( player2.hand[i].left == f.left ) and ( player2.hand[i].right == f.right ))
				{
					player2.hand.erase(player2.hand.begin() + i);
					erase = true;
				}

				else if(( player2.hand[i].left == f.right ) and ( player2.hand[i].right == f.left ))
				{
					player2.hand.erase(player2.hand.begin() + i);
					erase = true;
				}
			}
		}
	}


};
