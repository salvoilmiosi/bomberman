#include "vote_handler.h"

#include "game_server.h"

vote_handler::vote_handler(game_server *server) : server(server) {}

void vote_handler::tick() {
    if (vote_start_time > 0 && SDL_GetTicks() - vote_start_time > VOTE_TIMER) {
        reset();
        server->messageToAll(COLOR_MAGENTA, "Vote failed.");
    }
}

void vote_handler::reset() {
    current_vote = 0;
    vote_start_time = 0;
    votes.clear();
}

void vote_handler::sendVote(user *u, uint8_t vote_type) {
    if (current_vote == 0) {
        current_vote = vote_type;
        vote_start_time = SDL_GetTicks();
    } else if (current_vote != vote_type) {
        packet_ext packet = server->messagePacket(COLOR_MAGENTA, "Another vote is up right now");
        server->sendRepeatPacket(packet, u->getAddress(), 5);
        return;
    }
    if (votes.find(u) == votes.end()) {
        votes[u] = vote_type;
    } else {
        packet_ext packet = server->messagePacket(COLOR_MAGENTA, "You already voted");
        server->sendRepeatPacket(packet, u->getAddress(), 5);
        return;
    }

    int num_users = server->countUsers(false);
    switch (vote_type) {
    case VOTE_START:
        server->messageToAll(COLOR_MAGENTA, "%s votes to start the game (%d / %d)", u->getName(), votes.size(), num_users);
        break;
    case VOTE_STOP:
        server->messageToAll(COLOR_MAGENTA, "%s votes to stop the server (%d / %d)", u->getName(), votes.size(), num_users);
        break;
    case VOTE_RESET:
        server->messageToAll(COLOR_MAGENTA, "%s votes to reset the game (%d / %d)", u->getName(), votes.size(), num_users);
        break;
    }
    if (int(votes.size()) == num_users) {
        switch (current_vote) {
        case VOTE_START:
            server->startGame();
            break;
        case VOTE_STOP:
            server->closeServer();
            break;
        case VOTE_RESET:
            server->resetGame();
            break;
        }
        reset();
    }
}
