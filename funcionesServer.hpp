
#include "message.hpp"

//------MACROS----------
#define MSG_SIZE 250
#define MAX_CLIENTS 30
#define MAX_GAMES 10
//----------------------

void salirCliente(int socket, fd_set * readfds, int * numClientes, std::vector<cliente> &arrayClientes);

void manejador (int signum);

bool checkUser(char buffer[]);

bool registerUser(char rUser[], char rPasswd[]);

bool verifyPassword(char cUser[], char cPasswd[]);

int matchmaking(std::vector<domino> games);

void alternatePlayerStatus(domino& game, std::vector<cliente>& arrayClientes);

bool canPutPiece(int i, domino& game);

bool userInWaitList(int sd, std::vector <int> waitList);

void managePostGame(std::vector<domino> &partida, int pos, std::vector<cliente> &arrayClientes, int numClientes, int *numPartidas);

void waitListGame(std::vector <int> &lista_espera, std::vector<domino> &partida, int pos, std::vector<cliente> &arrayClientes, int numClientes, int *numPartidas);

bool checkPiece(int i, char* buffer, ficha &auxFicha, char* &extremo);

bool puttingpieceP1(domino& game, ficha &auxFicha, char* extremo);

bool puttingpieceP2(domino& game, ficha &auxFicha, char* extremo);
