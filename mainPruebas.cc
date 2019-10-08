#include "domino.cc"

using namespace std;

int main()
{
	domino tablero;
	std::vector<ficha> mano;
	int opcion = -1;
	int x = -1;
	char aux;

	while(opcion != 0)
	{
		system("clear");
		
		std::cout << "---------------------------\n\n" ;

		std::cout << "Indique la accion a realizar:\n\n";

		std::cout << "1. Nueva partida.\n";
		std::cout << "2. Nueva mano.\n";
		std::cout << "3. Mostrar tablero.\n";
		std::cout << "4. Ver primero y ultimo.\n";
		std::cout << "5. Ver numero de piezas para robar.\n";
		std::cout << "6. Robar ficha.\n";
		std::cout << "7. Colocar ficha\n\n";

		std::cout << "0. Salir.\n\n";
		
		std::cout << "---------------------------\n\n";

		std::cout << "Opcion: ";
		std::cin >> opcion;

		switch(opcion)
		{
			case 0:
				system("clear");
				std::cout << "\n> Saliendo...\n\n";

			break;

			case 1:
				system("clear");
				std::cout << "1. Nueva partida\n\n";

				tablero.emptyBoard();

				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();

			break;

			case 2:
				system("clear");
				std::cout << "2. Nueva mano\n\n";

				mano = tablero.newHand();

				cout << "> Nueva mano establecida.\n\n";

				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();
			break;

			case 3:
				system("clear");
				std::cout << "3. Mostrar tablero\n\n";

				tablero.showBoard();

				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();
			break;

			case 4:
				system("clear");
				std::cout << "4. Ver primero y ultimo.\n\n";

				cout << "> Primero: " << tablero.getFirst() << "\n";
				cout << "> Ultimo: " << tablero.getLast() << "\n\n";

				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();
			break;

			case 5:
				system("clear");
				std::cout << "5. Ver numero de piezas para robar.\n\n";

				cout << "> Numero de piezas para robar: " << tablero.getPiecesForStole() << "\n\n";
			
				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();
			break;

			case 6:
				system("clear");
				std::cout << "6. Robar ficha.\n\n";

				if (tablero.isForStoleEmpty())
				{
					cout << "No quedan fichas para robar.\n\n";
				}

				else
				{
					mano.push_back(tablero.stealPiece());

					cout << "> Ficha robada: |" << mano[mano.size()-1].left << " · " << mano[mano.size()-1].right << "|\n\n";

					cout << "> Quedan " << tablero.getPiecesForStole() << " fichas para robar\n\n";
				}


				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();
			break;

			case 7:
				system("clear");
				std::cout << "7. Colocar ficha.\n\n";

				tablero.showBoard();

				cout << "¿Que ficha va a colocar?\n\n";

				for (int i = 0; i < (int) mano.size(); i++)
				{
					cout << i << "). |" << mano[i].right << " · " << mano[i].left << "| \n";
				}

				cout << "\n> Ficha: ";
				cin >> x;

				if (( x >= 0 ) and ( x < (int) mano.size() ))
				{
					if (tablero.putInBoard(mano[x]))
					{
						cout << "Ficha colocada.\n\n";
						mano.erase( mano.begin() + x );
					}

					else
					{
						cout << "Ficha invalida.\n\n";
					}
				}

				else
				{
					cout << "> Opcion invalida.\n\n";
				}
	
				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				std::cin.get(aux);
				std::cin.ignore();

			break;

			default:
				system("clear");
				std::cout << "\n> ERROR. Opcion no valida (" << opcion << ")\n\n";
		
				std::cout << "--------< PULSE INTRO PARA CONTINUAR >--------\n";
			
				cin.get(aux);
				cin.ignore();


		}
	}
}