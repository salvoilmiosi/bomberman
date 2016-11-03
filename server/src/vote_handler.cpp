#include "vote_handler.h"

#include "game_server.h"

vote_handler::vote_handler(game_server *server) : server(server) {}

void vote_handler::tick() {
    if (vote_start_time > 0 && SDL_GetTicks() - vote_start_time > VOTE_TIMER) {
        reset();
        server->messageToAll(COLOR_MAGENTA, "Not enough players voted.");
    }
}

void vote_handler::reset() {
    current_vote = 0;
    vote_start_time = 0;
    votes.clear();
}

void vote_handler::sendVote(user *u, uint32_t vote_type, uint32_t args) {
    if (!u->getPlayer()) {
        server->sendMessageTo(u->getAddress(), COLOR_MAGENTA, "Spectators can't vote.");
        return;
    }
    if (current_vote == 0) {
        if (vote_type == VOTE_YES || vote_type == VOTE_NO) {
            server->sendMessageTo(u->getAddress(), COLOR_MAGENTA, "No vote is up right now.");
            return;
        }

        vote_args = args;
        if (votes.find(u->getID()) == votes.end()) {
            switch (vote_type) {
            case VOTE_START:
                if (!server->gameStarted()) {
                    server->messageToAll(COLOR_MAGENTA, "%s votes to start the game.", u->getName());
                } else {
                    server->sendMessageTo(u->getAddress(), COLOR_MAGENTA, "Game has already started.");
                    return;
                }
                break;
            case VOTE_STOP:
                server->messageToAll(COLOR_MAGENTA, "%s votes to stop the server.", u->getName());
                break;
            case VOTE_RESET:
                server->messageToAll(COLOR_MAGENTA, "%s votes to reset the game.", u->getName());
                break;
            case VOTE_ADD_BOT:
                server->messageToAll(COLOR_MAGENTA, "%s votes to add %d bots.", u->getName(), args);
                break;
            case VOTE_REMOVE_BOTS:
                server->messageToAll(COLOR_MAGENTA, "%s votes to remove all bots.", u->getName());
                break;
            case VOTE_KICK:
            {
                user *kick_user = server->findUserByID(args);
                if (kick_user) {
                    server->messageToAll(COLOR_MAGENTA, "%s votes to kick %s.", u->getName(), kick_user->getName());
                } else {
                    server->sendMessageTo(u->getAddress(), COLOR_MAGENTA, "Invalid user id %d.", args);
                    return;
                }
                break;
            }
            case VOTE_ZONE:
                server->messageToAll(COLOR_MAGENTA, "%s votes to change the zone to %s.", u->getName(), game_map::getZoneName(static_cast<map_zone>(args)));
                break;
            default:
                server->sendMessageTo(u->getAddress(), COLOR_MAGENTA, "You started an incorrect vote.");
                return;
            }

            current_vote = vote_type;
            vote_start_time = SDL_GetTicks();
            votes[u->getID()] = VOTE_YES;
        }
    } else if (vote_type == VOTE_YES || vote_type == VOTE_NO) {
        votes[u->getID()] = vote_type;
        server->messageToAll(COLOR_MAGENTA, "%s voted %s.", u->getName(), vote_type == VOTE_YES ? "yes" : "no");
    } else if (vote_type == current_vote) {
        votes[u->getID()] = VOTE_YES;
        server->messageToAll(COLOR_MAGENTA, "%s voted yes.", u->getName());
    } else if (current_vote != vote_type) {
        packet_ext packet = server->messagePacket(COLOR_MAGENTA, "Another vote is up right now.");
        server->sendRepeatPacket(packet, u->getAddress());
        return;
    }

    uint32_t num_users = server->countUsers(false);
    uint32_t num_yes = 0;
    uint32_t num_no = 0;
    for (auto it : votes) {
        if (it.second == VOTE_YES) ++num_yes;
        else if (it.second == VOTE_NO) ++num_no;
    }

    if (num_yes > num_users / 2) {
        if (num_users > 1) {
            server->messageToAll(COLOR_MAGENTA, "Yes: %d, No: %d - Vote succeeded.", num_yes, num_no);
        }
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
        case VOTE_ADD_BOT:
            server->addBots(vote_args);
            break;
        case VOTE_REMOVE_BOTS:
            server->removeBots();
            break;
        case VOTE_KICK:
            server->kickUser(server->findUserByID(vote_args), "You have been voted off.");
            break;
        case VOTE_ZONE:
            server->setZone(static_cast<map_zone>(args));
            break;
        }
        reset();
    } else if (num_yes + num_no >= num_users) {
        reset();
        server->messageToAll(COLOR_MAGENTA, "Yes: %d, No: %d - Vote failed.", num_yes, num_no);
    }
}
