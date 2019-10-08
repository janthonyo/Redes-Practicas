#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <ctime>
#include <netdb.h>
#include <arpa/inet.h>

struct ficha
{
	int left;
	int right;
};

class domino
{
private:
	std::vector<ficha> board;
	std::vector<ficha> forStole; //Monton para robar 
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

	int getFirst(){return first;}

	int getLast(){return last;}

	int getPiecesForStole(){return (int) forStole.size();}

	bool isForStoleEmpty(){return (forStole.empty());}

	bool isBoardEmpty(){return (board.empty());}

	void showBoard()
	{
		if (isBoardEmpty())
		{
			std::cout << "El tablero esta vacio.";
		}

		else
		{
			for (int i = 0; i < (int) board.size(); i++)
			{
				std::cout << "|" << board[i].left << " · " << board[i].right << "|";
			}
		}

		std::cout << "\n\n";
	}


	//Modificadores <--------------------------------------------------

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
	
};