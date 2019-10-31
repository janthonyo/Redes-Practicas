#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

#include "message.hpp"

void sendBoth(domino& game, char mensaje[MSG_SIZE])
{
    send(game.getSocketP1(), mensaje, strlen(mensaje), 0);
    send(game.getSocketP2(), mensaje, strlen(mensaje), 0);
}

void sendBoardAndHands(domino& game)
{
    char mensaje[MSG_SIZE];

    // Enviamos el tablero
    strcpy(mensaje, game.showBoard().c_str());
    sendBoth(game, mensaje);

    // Send hands
    strcpy(mensaje, game.messageHandP1().c_str());
    send(game.getSocketP1(), mensaje, strlen(mensaje), 0);

    strcpy(mensaje, game.messageHandP2().c_str());
    send(game.getSocketP2(), mensaje, strlen(mensaje), 0);
}

void nextTurnMessage(int *i, domino& game)
{
    char mensaje[MSG_SIZE];

    //Se informa de paso de turno
    if(*i == game.getSocketP1()){
        strcpy(mensaje, "+Ok. Turno de partida.\n\n");
        send(game.getSocketP2(), mensaje, strlen(mensaje), 0);

        strcpy(mensaje, "+Ok. Turno del otro jugador.\n\n");
        send(game.getSocketP1(), mensaje, strlen(mensaje), 0);
    }
    else if (*i == game.getSocketP2()) {
        strcpy(mensaje, "+Ok. Turno de partida.\n\n");
        send(game.getSocketP1(), mensaje, strlen(mensaje), 0);

        strcpy(mensaje, "+Ok. Turno del otro jugador.\n\n");
        send(game.getSocketP2(), mensaje, strlen(mensaje), 0);
    }
}

void startGameMessage(domino& game, int result, int *numClientes, std::vector<cliente> &arrayClientes)
{
    //Var auxiliares
    char mensaje[MSG_SIZE];
    char buffer[MSG_SIZE];

    //Enviamos los jugadores
    strcpy(mensaje, "\nJUGADORES:\n");
    strcat(mensaje, "\tJugador1(");
    strcat(mensaje, game.getUserP1());
    strcat(mensaje, ")\n");
    strcat(mensaje, "\tJugador2(");
    strcat(mensaje, game.getUserP2());
    strcat(mensaje, ")\n\n");
    sendBoth(game, mensaje);

    // Turno del J1 / Espera J2
    if (result == 1) {
        //Enviamos que jugador ha sacado
        strcpy(mensaje, "+Ok. Jugador2 sale y ha colocado ficha.\n\n");
        sendBoth(game, mensaje);

        // Enviamos los turnos
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "+Ok. Turno de partida.\n");
        send(game.getSocketP1(), buffer, strlen(buffer), 0);

        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "+Ok. Turno del otro jugador.\n");
        send(game.getSocketP2(), buffer, strlen(buffer), 0);

        // Cambiamos los estados de los jugadores
        for(int j = 0; j < *numClientes; j++)
        {
            if (arrayClientes[j].sd == game.getSocketP1())
                arrayClientes[j].status = 4;        // J1 Puede colocar

            else if (arrayClientes[j].sd == game.getSocketP2())
                arrayClientes[j].status = 3;        // J2 espera
        }
    }
    // Turno de J2 / Espera J1
    else {
        //Enviamos que jugador ha sacado
        strcpy(mensaje, "+Ok. Jugador1 sale y ha colocado ficha.\n\n");
        sendBoth(game, mensaje);

        // Enviamos los turnos
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "+Ok. Turno del otro jugador.\n");
        send(game.getSocketP1(), buffer, strlen(buffer), 0);

        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "+Ok. Turno de partida.\n");
        send(game.getSocketP2(), buffer, strlen(buffer), 0);

        // Cambiamos los estados de los jugadores
        for(int j = 0; j < *numClientes; j++)
        {
            if (arrayClientes[j].sd == game.getSocketP1())
                arrayClientes[j].status = 3;    // J1 Espera

            else if (arrayClientes[j].sd == game.getSocketP2())
                arrayClientes[j].status = 4;    // J2 Puede colocar
        }
    }

    sendBoardAndHands(game);
}

void cantPutPieceMessage(int i)
{
    char buffer[MSG_SIZE];

    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "-ERR. La ficha no puede ser colocada.\n");
    send(i, buffer, strlen(buffer), 0);
}
