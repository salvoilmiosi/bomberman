#include "chat.h"
#include "resources.h"
#include "main.h"
#include "game_client.h"
#include "game_sound.h"

chat::~chat() {
    for(chat_line &l : lines) {
        if (l.texture) {
            SDL_DestroyTexture(l.texture);
        }
    }
    lines.clear();
}

void chat::addLine(uint32_t color, const char *line) {
    chat_line l = {std::string(line), color, SDL_GetTicks(), nullptr, 0};
    lines.push_back(l);
    if (lines.size() > MAX_LINES) {
        lines.pop_front();
    }
    playWave(WAV_SELECT);
}

void chat::tick() {
    if (lines.empty()) return;

    chat_line &line = lines.front();
    if (SDL_GetTicks() - line.time > LINE_LIFE) {
        if (line.texture) {
            SDL_DestroyTexture(line.texture);
        }
        lines.pop_front();
    }
}

#ifdef _SDL_TTF_H

extern TTF_Font *resource_font;
void renderLine(int ox, int oy, SDL_Renderer *renderer, chat::chat_line &line);

#endif // _SDL_TTF_H

void chat::render(SDL_Renderer *renderer) {
    int x = CHAT_XOFFSET;
    int y = WINDOW_HEIGHT - CHAR_H - CHAT_YOFFSET;

    if (is_typing) {
        std::string str = typing_text;
        int time_since_typing = SDL_GetTicks() - typing_time;
        if ((time_since_typing / 500) % 2 == 0) {
#ifdef _SDL_TTF_H
            str.append("\u25a0"); // black square character
            //str.append("\u0028\u0020\u0361\u00b0\u0020\u035c\u0296\u0020\u0361\u00b0\u0029"); // lenny face, because why not
            //str.append("\U0001f4a9"); // pile of poop, because I can
            //str.append("\U0001f600"); // smiley face
#else
            str.append("|");
#endif
        }
        renderText(x, y, renderer, str, 0xffffffff);
    }

    y -= CHAR_H + CHAT_LINE_SPACE;

    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
        chat_line &l = *it;
        uint8_t alpha = 255.f * (1.f - float(SDL_GetTicks() - l.time) / LINE_LIFE);
        l.color = (l.color & 0xffffff00) | alpha;

#ifdef _SDL_TTF_H
        renderLine(x, y, renderer, l);
#else
        renderText(x, y, renderer, l.message, l.color);
#endif // _SDL_TTF_H

        y -= CHAR_H + CHAT_LINE_SPACE;
    }
}

void chat::startTyping() {
    is_typing = true;
    typing_text.clear();
    SDL_StartTextInput();

    typing_time = SDL_GetTicks();
}

void chat::stopTyping() {
    if (typing_text.empty()) return;

    if (typing_text[0] == '/') {
        client->execCmd(typing_text.substr(1));
    } else {
        client->sendChatMessage(typing_text.c_str());
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
            while (! typing_text.empty()) {
                uint8_t last_char = typing_text.back();
                typing_text.pop_back();
                if (last_char < 0x80) {
                    break;
                }
                // The first byte of a UTF-8 character is 11XXXXXX
                if ((last_char & 0xc0) == 0xc0) {
                    break;
                }
            }
            break;
        default:
            break;
        }
        break;
    case SDL_TEXTINPUT:
        typing_text.append(e.text.text);
        break;
    }
}

#define _SDL_TTF_H
#ifdef _SDL_TTF_H
SDL_Texture *createTextTexture(SDL_Renderer *renderer, const std::string &text, SDL_Rect *clip_rect) {
    static const SDL_Color white = {0xff, 0xff, 0xff};
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(resource_font, text.c_str(), white);

    if (!text_surface) return nullptr;

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    *clip_rect = text_surface->clip_rect;

    SDL_FreeSurface(text_surface);

    return text_texture;
}

void renderTextTexture(int ox, int oy, SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect src_rect, uint32_t color, int align) {
    uint8_t r = (color & 0xff000000) >> (8 * 3);
    uint8_t g = (color & 0x00ff0000) >> (8 * 2);
    uint8_t b = (color & 0x0000ff00) >> (8 * 1);
    uint8_t a = (color & 0x000000ff) >> (8 * 0);

    SDL_SetTextureAlphaMod(texture, a);

    SDL_Rect dst_rect = src_rect;
    dst_rect.x = ox + SHADOW_OFFSET;
    dst_rect.y = oy + SHADOW_OFFSET;

    switch(align) {
    case ALIGN_MIDDLE:
        dst_rect.x -= dst_rect.w / 2;
        break;
    case ALIGN_RIGHT:
        dst_rect.x -= dst_rect.w;
        break;
    }

    SDL_SetTextureColorMod(texture, 0, 0, 0);
    SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);

    dst_rect.x -= SHADOW_OFFSET;
    dst_rect.y -= SHADOW_OFFSET;

    SDL_SetTextureColorMod(texture, r, g, b);
    SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);
}

void renderLine(int ox, int oy, SDL_Renderer *renderer, chat::chat_line &line) {
    if (line.texture == nullptr) {
        line.texture = createTextTexture(renderer, line.message, &line.clip_rect);
    }

    SDL_Rect dst_rect = line.clip_rect;
    dst_rect.x += ox;
    dst_rect.y += oy;

    uint8_t a = (line.color & 0x000000ff) >> (8 * 0) * 0x80 / 0xff;
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &dst_rect);

    renderTextTexture(ox, oy, renderer, line.texture, line.clip_rect, line.color, ALIGN_LEFT);
}

void renderText(int ox, int oy, SDL_Renderer *renderer, const std::string &text, uint32_t color, int align) {
    SDL_Rect src_rect;
    SDL_Texture *text_texture = createTextTexture(renderer, text, &src_rect);
    renderTextTexture(ox, oy, renderer, text_texture, src_rect, color, align);
    SDL_DestroyTexture(text_texture);
}

#else
void renderText(int ox, int oy, SDL_Renderer *renderer, const std::string &text, uint32_t color, int align) {
    int len = text.size();

    SDL_Rect src_rect = {0, 0, CHAR_W, CHAR_H};
    SDL_Rect dst_rect = {0, 0, CHAR_W, CHAR_H};

    uint8_t r = (color & 0xff000000) >> (8 * 3);
    uint8_t g = (color & 0x00ff0000) >> (8 * 2);
    uint8_t b = (color & 0x0000ff00) >> (8 * 1);
    uint8_t a = (color & 0x000000ff) >> (8 * 0);

    switch(align) {
    case ALIGN_MIDDLE:
        ox -= CHAR_W * len / 2;
        break;
    case ALIGN_RIGHT:
        ox -= CHAR_W * len;
        break;
    }

    for (int i=0; i<len; ++i) {
        uint8_t c = text[i];
        int tx = c % 16;
        int ty = c / 16;
        src_rect.x = tx * CHAR_W;
        src_rect.y = ty * CHAR_H;
        dst_rect.x = ox + i * CHAR_W + 2;
        dst_rect.y = oy + 2;
        SDL_SetTextureColorMod(font_texture, 0, 0, 0);
        SDL_SetTextureAlphaMod(font_texture, a);
        SDL_RenderCopy(renderer, font_texture, &src_rect, &dst_rect);

        dst_rect.x -= 2;
        dst_rect.y -=2;
        SDL_SetTextureColorMod(font_texture, r, g, b);
        SDL_RenderCopy(renderer, font_texture, &src_rect, &dst_rect);
    }
}
#endif
