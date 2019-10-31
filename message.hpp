#include "domino.cc"


void sendBoth(domino& game, char mensaje[MSG_SIZE]);

void sendBoardAndHands(domino& game);

void nextTurnMessage(int *i, domino& game);

void startGameMessage(domino& game, int result, int *numClientes, std::vector<cliente> &arrayClientes);

void cantPutPieceMessage(int i);
