#include "score.h"

#include "game_client.h"

#include "main.h"
#include "chat.h"

#include <cstring>

score::score(game_client *client) : client(client) {
    is_shown = false;

    ticks_to_send_score = 0;
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

    char buffer[16];

    for (int i=0; i<num_players; ++i) {
        score_info si = info[i];

        if (si.is_player) {
            renderText(x, y, renderer, itoa(si.player_num, buffer, 10), 0xffffffff);
            renderText(x + 200, y, renderer, itoa(si.victories, buffer, 10), 0xffffffff);
        } else {
            renderText(x + 200, y, renderer, "Spectator", 0xffff00ff);
        }
        renderText(x + 100, y, renderer, si.player_name, 0xffffffff);
        renderText(x + 300, y, renderer, itoa(si.ping, buffer, 10), 0xffffffff);

        y += CHAR_H + CHAT_LINE_SPACE;
    }
}

void score::show(bool shown) {
    is_shown = shown;
}

void score::handlePacket(packet_ext &packet) {
    int i=0;
    while (!packet.atEnd() && i < MAX_INFO_SIZE) {
        score_info &in = info[i];

        const char *player_name = packet.readString();
        strncpy(in.player_name, player_name, NAME_SIZE);

        in.ping = packet.readShort();
        in.is_player = packet.readChar() != 0;
        if (in.is_player) {
            in.player_num = packet.readChar();
            in.victories = packet.readShort();
        }
        ++i;
    }

    num_players = i;
}
