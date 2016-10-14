#include "chat.h"
#include "resources.h"
#include "main.h"
#include "game_client.h"

#include <cstring>
#include <string>

void chat::addLine(uint32_t color, const char *line) {
    chat_line l = {std::string(line), color, SDL_GetTicks()};
    lines.push_back(l);
    if (lines.size() > MAX_LINES) {
        lines.pop_front();
    }
}

void chat::tick() {
    if (lines.empty()) return;
    if (SDL_GetTicks() - lines.front().time > LINE_LIFE) {
        lines.pop_front();
    }
}

void chat::render(SDL_Renderer *renderer) {
    int x = CHAT_XOFFSET;
    int y = WINDOW_HEIGHT - CHAR_H - CHAT_YOFFSET;

    if (is_typing) {
        char str[CHAT_TYPING_SIZE];
        strncpy(str, typing, CHAT_TYPING_SIZE);
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            strncat(str, "|", CHAT_TYPING_SIZE);
        }
        renderText(x, y, renderer, str, 0xffffffff);
    }

    y -= CHAR_H + CHAT_LINE_SPACE;

    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
        const chat_line &l = *it;
        uint32_t color = l.color;
        uint8_t alpha = 255.f * (1.f - float(SDL_GetTicks() - l.time) / LINE_LIFE);
        color = (color & 0xffffff00) | alpha;

        renderText(x, y, renderer, l.message.c_str(), color);

        y -= CHAR_H + CHAT_LINE_SPACE;
    }
}

void chat::startTyping() {
    is_typing = true;
    memset(typing, 0, CHAT_TYPING_SIZE);
    SDL_StartTextInput();
}

void chat::stopTyping() {
    if (*typing == '/') {
        std::string message = typing;

        size_t space_begin = 1;
        size_t space_in = message.find(' ', space_begin);

        std::string msg = message.substr(space_begin, space_in - space_begin);
        if (msg == "connect") {
            std::string address, port;

            if (space_in == std::string::npos) {
                addLine(COLOR_ORANGE, "Usage: connect address [port]");
                return;
            }

            space_begin = space_in + 1;
            space_in = message.find(' ', space_begin);
            address = message.substr(space_begin, space_in - space_begin);

            if (space_in == std::string::npos) {
                client->connect(address.c_str());
                return;
            }
            space_begin = space_in + 1;
            space_in = message.find(' ', space_begin);
            port = message.substr(space_begin, space_in - space_begin);

            client->connect(address.c_str(), atoi(port.c_str()));
        } else if (msg == "disconnect") {
            client->disconnect();
        } else if (msg == "name") {
            const char *name;

            if (space_in == std::string::npos) {
                addLine(COLOR_ORANGE, "Usage: name new_name");
                return;
            }
            space_begin = space_in + 1;
            space_in = message.find(' ', space_begin);
            name = message.substr(space_begin, space_in - space_begin).c_str();

            client->setName(name);
        } else if (msg == "music_volume") {
            if (space_in == std::string::npos) {
                addLine(COLOR_ORANGE, "Usage: music_volume [0-100]");
                return;
            }

            space_begin = space_in + 1;
            space_in = message.find(' ', space_begin);
            client->setMusicVolume(message.substr(space_begin, space_in - space_begin).stoi() * MIX_MAX_VOLUME / 100);
        }
    } else {
        client->sendChatMessage(typing);
    }
}

void chat::handleEvent(const SDL_Event &e) {
    switch (e.type) {
    case SDL_KEYDOWN:
        switch (e.key.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE:
            is_typing = false;
            SDL_StopTextInput();
            break;
        case SDL_SCANCODE_RETURN:
            is_typing = false;
            SDL_StopTextInput();
            stopTyping();
            break;
        case SDL_SCANCODE_BACKSPACE:
            if (*typing != '\0') {
                typing[strlen(typing) - 1] = '\0';
            }
            break;
        default:
            break;
        }
        break;
    case SDL_TEXTINPUT:
        strncat(typing, e.text.text, CHAT_TYPING_SIZE);
        break;
    }
}

void renderText(int ox, int oy, SDL_Renderer *renderer, const char *text, uint32_t color) {
    int len = strlen(text);
    SDL_Rect src_rect = {0, 0, CHAR_W, CHAR_H};
    SDL_Rect dst_rect = {0, 0, CHAR_W, CHAR_H};

    uint8_t r = (color & 0xff000000) >> (8 * 3);
    uint8_t g = (color & 0x00ff0000) >> (8 * 2);
    uint8_t b = (color & 0x0000ff00) >> (8 * 1);
    uint8_t a = (color & 0x000000ff) >> (8 * 0);

    for (int i=0; i<len; ++i) {
        char c = text[i];
        int tx = c % 16;
        int ty = c / 16;
        src_rect.x = tx * CHAR_W;
        src_rect.y = ty * CHAR_H;
        dst_rect.x = ox + i * CHAR_W + 2;
        dst_rect.y = oy + 2;
        SDL_SetTextureColorMod(text_texture, 0, 0, 0);
        SDL_SetTextureAlphaMod(text_texture, a);
        SDL_RenderCopy(renderer, text_texture, &src_rect, &dst_rect);

        dst_rect.x -= 2;
        dst_rect.y -=2;
        SDL_SetTextureColorMod(text_texture, r, g, b);
        SDL_RenderCopy(renderer, text_texture, &src_rect, &dst_rect);
    }
}
