#ifndef __CHAT_H__
#define __CHAT_H__

#include <SDL2/SDL.h>
#include <deque>
#include <string>

static const size_t MAX_LINES = 10;

static const int CHAR_W = 8;
static const int CHAR_H = 14;

static const int CHAT_XOFFSET = 20;
static const int CHAT_YOFFSET = 20;
static const int CHAT_LINE_SPACE = 2;

static const int CHAT_TYPING_SIZE = 128;

static const int LINE_LIFE = 10000;

class chat {
private:
    struct chat_line {
        std::string message;
        uint32_t color;
        uint32_t time;
    };

    std::deque<chat_line> lines;

    bool is_typing = false;

    char typing[CHAT_TYPING_SIZE];

    class game_client *client;

public:
    chat(class game_client *client) : client(client) {}

public:
    void addLine(const char *line, uint32_t color);

    void tick();

    void render(SDL_Renderer *renderer);

    void startTyping();

    bool isTyping() {
        return is_typing;
    }

    void handleEvent(const SDL_Event &e);
};

void renderText(int x, int y, SDL_Renderer *renderer, const char *text, uint32_t color);

#endif // __CHAT_H__
