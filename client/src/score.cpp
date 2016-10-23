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
        ticks_to_send_score = TICKRATE / 4;
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
    int h = num_players * CHAR_H + (num_players - 1) * SCORE_LINE_SPACE + 40;

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

        int icon_y = y - (ICON_SIZE - CHAR_H) / 2;

        if (si.user_type != SCORE_SPECTATOR && si.player_num <= 3) {
            SDL_Rect src_rect = {si.player_num * 16, si.alive ? 0 : 16, 16, 16};
            SDL_Rect dst_rect = {x, icon_y, ICON_SIZE, ICON_SIZE};
            SDL_RenderCopy(renderer, player_icons_texture, &src_rect, &dst_rect);
        }

        bool is_self = si.user_type != SCORE_BOT && si.user_id == self_id;

        renderText(x + 40, y, renderer, si.player_name, is_self ? COLOR_YELLOW : COLOR_WHITE);
        if (si.user_type == SCORE_SPECTATOR) {
            renderText(x + 330, y, renderer, "SPEC", COLOR_YELLOW, ALIGN_RIGHT);
        } else {
            int trophy_size = ICON_SIZE;
            SDL_Rect src_rect = {0, 48, 16, 16};
            if (si.victories > 5) {
                trophy_size = ICON_SIZE / 2;
            }
            if (si.victories > 20) {
                icon_y = y - (trophy_size - CHAR_H) / 2;
                SDL_Rect dst_rect = {x + 330 - trophy_size, icon_y, trophy_size, trophy_size};
                renderText(x + 330 - trophy_size - 5, y, renderer, std::to_string(si.victories) + " x", COLOR_WHITE, ALIGN_RIGHT);
                SDL_RenderCopy(renderer, player_icons_texture, &src_rect, &dst_rect);
            } else {
                if (si.victories <= 10) {
                    icon_y = y - (trophy_size - CHAR_H) / 2;
                }
                SDL_Rect dst_rect = {x + 330 - trophy_size, icon_y, trophy_size, trophy_size};
                for (int i=0; i<si.victories; ++i) {
                    SDL_RenderCopy(renderer, player_icons_texture, &src_rect, &dst_rect);
                    dst_rect.x -= trophy_size + 5;
                    if (dst_rect.x < x + 120) {
                        dst_rect.x = x + 330 - trophy_size;
                        dst_rect.y += trophy_size;
                    }
                }
            }
        }
        if (si.user_type == SCORE_BOT) {
            renderText(x + 350, y, renderer, "BOT", COLOR_YELLOW);
        } else if (si.ping >= 0) {
            renderText(x + 350, y, renderer, std::to_string(si.ping), COLOR_WHITE);
        } else {
            renderText(x + 350, y, renderer, "---", COLOR_YELLOW);
        }

        y += CHAR_H + SCORE_LINE_SPACE;
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
            in.user_id = packet.readShort();
            in.player_num = packet.readChar();
            in.victories = packet.readShort();
            in.alive = packet.readChar() > 0;
            break;
        case SCORE_SPECTATOR:
        	in.victories = 0;
        	in.user_id = packet.readShort();
        	break;
        default:
            break;
        }
        ++i;
    }

    num_players = i;

    std::sort(std::begin(info), std::begin(info) + num_players, compare);
}

int score::findUserID(const char *name) {
    for (int i=0; i<num_players; ++i) {
        if (strcasecmp(name, info[i].player_name) == 0) {
            return info[i].user_id;
        }
    }
    return -1;
}
