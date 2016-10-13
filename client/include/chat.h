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

void renderText(int x, int y, SDL_Renderer *renderer, const char *text, uint32_t color);

#endif // __CHAT_H__
