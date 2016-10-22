#include "score.h"

#include "game_client.h"

#include "main.h"
#include "chat.h"
#include "resources.h"

#include <string>
#include <cstring>
#include <algorithm>

score::score(game_client *client) : client(client) {
    is_shown = false;

    ticks_to_send_score = 0;

    clear();
}

void score::clear() {
    num_players = 0;

    memset(info, 0, sizeof(info));
}

void score::tick() {
    if (ticks_to_send_score <= 0) {
        client->sendScorePacket();
        ticks_to_send_score = TICKRATE;
    } else {
        --ticks_to_send_score;
    }
}

bool score::score_info_compare::operator()(const score_info &a, const score_info &b) {
    if (a.user_type == SCORE_SPECTATOR && b.user_type != SCORE_SPECTATOR) {
        return false;
    } else if (a.user_type != SCORE_SPECTATOR && b.user_type == SCORE_SPECTATOR) {
        return true;
    } else if (a.user_type == SCORE_SPECTATOR && b.user_type == SCORE_SPECTATOR) {
        return strcmp(a.player_name, b.player_name) < 0;
    } else if (a.victories == b.victories) {
    	return a.player_num < b.player_num;
    } else {
    	return a.victories > b.victories;
    }
}

void score::render(SDL_Renderer *renderer) {
    //if (num_players < 0) return;
    if (!is_shown) return;

    int w = SCOREBOARD_WIDTH;
    int h = num_players * CHAR_H + (num_players - 1) * CHAT_LINE_SPACE + 40;

    int x = (WINDOW_WIDTH - w) / 2;
    int y = (WINDOW_HEIGHT - h) / 2;

    SDL_Rect bg_rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0x80, 0xcc);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &bg_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    x += 20;
    y += 20;

    for (int i=0; i<num_players; ++i) {
        score_info si = info[i];

        static const char *player_num_name[] = {"WHITE", "BLACK", "BLUE", "RED"};

        if (si.user_type == SCORE_SPECTATOR || si.player_num > 3) {
            renderText(x, y, renderer, "---", COLOR_YELLOW);
        } else {
            renderText(x, y, renderer, player_num_name[si.player_num], si.alive ? COLOR_WHITE : COLOR_RED);
        }
        if (si.user_type == SCORE_SPECTATOR) {
            renderText(x + 200, y, renderer, "SPEC", COLOR_YELLOW);
        } else {
            renderText(x + 200, y, renderer, std::to_string(si.victories), COLOR_WHITE);
        }
        if (si.user_type == SCORE_BOT) {
            renderText(x + 300, y, renderer, "BOT", COLOR_YELLOW);
        } else if (si.ping >= 0) {
            renderText(x + 300, y, renderer, std::to_string(si.ping), COLOR_WHITE);
        } else {
            renderText(x + 300, y, renderer, "---", COLOR_YELLOW);
        }
        renderText(x + 100, y, renderer, si.player_name, COLOR_WHITE);

        y += CHAR_H + CHAT_LINE_SPACE;
    }
}

void score::show(bool shown) {
    is_shown = shown;
}

void score::handlePacket(byte_array &packet) {
    int i=0;
    while (!packet.atEnd() && i < MAX_INFO_SIZE) {
        score_info &in = info[i];

        const char *player_name = packet.readString();
        strncpy(in.player_name, player_name, NAME_SIZE);

        in.ping = packet.readShort();
        in.user_type = packet.readChar();
        switch (in.user_type) {
        case SCORE_PLAYER:
        case SCORE_BOT:
            in.player_num = packet.readChar();
            in.victories = packet.readShort();
            in.alive = packet.readChar() > 0;
            break;
        case SCORE_SPECTATOR:
        	in.victories = 0;
        	break;
        default:
            break;
        }
        ++i;
    }

    num_players = i;

    std::sort(std::begin(info), std::begin(info) + num_players, compare);
}
