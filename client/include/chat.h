#ifndef __CHAT_H__
#define __CHAT_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <deque>
#include <string>

#include "main.h"

static const size_t MAX_LINES = 10;

static const int CHAT_XOFFSET = 20;
static const int CHAT_YOFFSET = 20;
static const int CHAT_LINE_SPACE = 2;

static const int CHAT_TYPING_SIZE = 128;

static const int LINE_LIFE = 10 * TICKRATE;

#ifdef _SDL_TTF_H
static const int CHAR_H = 16;
#else
static const int CHAR_W = 8;
static const int CHAR_H = 14;
#endif

static const int SHADOW_OFFSET = 2;

static const uint32_t COLOR_RED        = 0xff0000ff;
static const uint32_t COLOR_GREEN      = 0x00ff00ff;
static const uint32_t COLOR_BLUE       = 0x0000ffff;
static const uint32_t COLOR_MAGENTA    = 0xff00ffff;
static const uint32_t COLOR_YELLOW     = 0xffff00ff;
static const uint32_t COLOR_CYAN       = 0x00ffffff;
static const uint32_t COLOR_BLACK      = 0x000000ff;
static const uint32_t COLOR_WHITE      = 0xffffffff;
static const uint32_t COLOR_ORANGE     = 0xff8000ff;

class chat {
public:
    struct chat_line {
        std::string message;
        uint32_t color;
        int uptime;
        SDL_Texture *texture;
        SDL_Rect clip_rect;
    };

private:
    std::deque<chat_line> lines;

    bool is_typing = false;

    std::string typing_text;

    class game_client *client;

    int typing_time;

public:
    chat(class game_client *client) : client(client) {}
    virtual ~chat();

public:
    template<typename ... T> void addLine(uint32_t color, const char *format, T ... args) {
        char line[CHAT_TYPING_SIZE];
        snprintf(line, CHAT_TYPING_SIZE, format, args ...);
        addLine(color, line);
    }

    void addLine(uint32_t color, const char *line);

    void tick();

    void render(SDL_Renderer *renderer);

    void startTyping();

    void stopTyping();

    bool isTyping() {
        return is_typing;
    }

    void handleEvent(const SDL_Event &e);
};

static const uint8_t ALIGN_LEFT = 0;
static const uint8_t ALIGN_RIGHT = 1;
static const uint8_t ALIGN_MIDDLE = 2;

void renderText(int x, int y, SDL_Renderer *renderer, const std::string &text, uint32_t color, int align = ALIGN_LEFT);

#endif // __CHAT_H__
