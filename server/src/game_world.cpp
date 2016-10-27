#include "game_world.h"

#include "player.h"

#include <cstring>

#include "ent_bomb.h"
#include "ent_item.h"
#include "main.h"

game_world::game_world(): server(this), g_map(this) {}

game_world::~game_world() {
    clear();
}

void game_world::clear() {
    for (auto ent : entities) {
        if (ent) delete ent;
    }
    entities.clear();
    g_map.clear();
}

int game_world::startServer(uint16_t port) {
    server.openServer(port);

    if (server.isOpen()) {
        printf("Server listening on port %d\n", port);
    } else {
        fprintf(stderr, "Could not open server socket: %s\n", SDLNet_GetError());
        return 1;
    }

    return server.run();
}

void game_world::addEntity(entity *ent) {
    if (!ent) return;

    entities.insert(ent);

    server.sendAddPacket(ent);
}

void game_world::removeEntity(entity *ent) {
    if (!ent) return;

    server.sendRemovePacket(ent);

    ent->destroy();
}

void game_world::tick() {
    if (!round_started) return;

    --ticks_to_start;

    if (ticks_to_start == 0) {
        server.messageToAll(COLOR_RED, "Round %d, fight!", round_num);
        countdownEnd();
    } else if (ticks_to_start > 0 && ticks_to_start % TICKRATE == 0) {
        int seconds = ticks_to_start / TICKRATE;
        server.messageToAll(COLOR_GREEN, "Round %d begins in %d seconds", round_num, seconds);
    }

    if (ticks_to_start < 0 && g_map.getZone() == ZONE_BOMB) {
        if ((-ticks_to_start) % (TICKRATE * 6) == 0) {
            int rand_x = random_engine() % (g_map.getWidth() - 4) + 2;
            int rand_y = random_engine() % (g_map.getHeight() - 2) + 1;
            addEntity(new bomb(this, rand_x, rand_y));
        }
    }

    int alive_players = 0;
    int num_players = 0;

    g_map.tick();

    auto it = entities.begin();
    while (it != entities.end()) {
        entity *ent = *it;
        if (ent && ent->isNotDestroyed()) {
            if (ent->getType() == TYPE_PLAYER) {
                player *p = dynamic_cast<player*>(ent);
                if (p->isAlive()) {
                    ++alive_players;
                }
                ++num_players;
            }
            ent->tick();
            ++it;
        } else {
            it = entities.erase(it);
            removeEntity(ent);
            if (ent) {
                delete ent;
            }
        }
    }

    bool round_end = alive_players <= (num_users == 1 ? 0 : 1);
    if (ticks_to_start < 0 && round_started && round_end) {
        if (alive_players == 0) {
            server.messageToAll(COLOR_RED, "DRAW!");
        }
        for (entity *ent : entities) {
            if (ent && ent->isNotDestroyed() && ent->getType() == TYPE_PLAYER) {
                player *p = dynamic_cast<player*>(ent);
                if (p->isAlive()) {
                    server.messageToAll(COLOR_RED, "%s won round %d!", p->getName(), round_num);
                    p->addVictory();
                    break;
                }
            }
        }
        round_started = false;
        if (num_players == 0) {
            // Everyone disconnected
            server.sendResetPacket();
            restartGame();
            clear();
        } else {
            startRound(num_players);
        }
    }
}

void game_world::writeEntitiesToPacket(packet_ext &packet, bool is_add) {
    for(entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && (is_add || ent->doSendUpdates())) {
            ent->writeToPacket(packet);
        }
    }
}

void game_world::sendKillMessage(player *killer, player *p) {
    static const std::vector<const char *> SUICIDE_MSG = {
        "%s couldn't handle it and killed himself.",
        "%s forgot that his own bombs hurt.",
        "%s has an explosive personality.",
        "%s obviously is not fireproof.",
        "%s clicked the big red button.",
        "%s bid farewell, cruel world!",
        "%s fears explosions too much.",
        "%s cut the wrong wire.",
        "%s never learned not to play with fire.",
    };

    static const std::vector<const char *> WORLD_DEATH_MSG = {
        "Life just isn't fair for %s.",
        "%s was killed by the world.",
        "%s was killed by the world. What a pity.",
        "%s had it coming for him.",
        "%s got hit by a stray bomb.",
        "%s is having a really bad day.",
        "That, right there, is a bomb, %s.",
    };

    static const std::vector<const char *> MURDER_MSG = {
        "%s was exploded by %s.",
        "%s's bits were sent everywhere by %s.",
        "%s was killed to death by %s.",
        "%s didn't pay enough attention to %s.",
        "%s died at the sight of %s.",
        "%s just wanted an hug from %s.",
        "%s thinks that %s is cheating.",
        "%s wondered what %s's bomb might be.",
    };

    if (p == killer) {
        server.messageToAll(COLOR_WHITE, SUICIDE_MSG[random_engine() % SUICIDE_MSG.size()], p->getName());
    } else if (killer == nullptr) {
        server.messageToAll(COLOR_WHITE, WORLD_DEATH_MSG[random_engine() % WORLD_DEATH_MSG.size()], p->getName());
    } else {
        server.messageToAll(COLOR_WHITE, MURDER_MSG[random_engine() % MURDER_MSG.size()], p->getName(), killer->getName());
    }
}

void game_world::restartGame() {
    round_started = false;
    round_num = 0;
    ticks_to_start = 0;
}

bool game_world::startRound(int num_u) {
    if (round_started) return false;
    if (ticks_to_start > 0) return false;

    num_users = num_u;

    ++round_num;

    ticks_to_start = TICKRATE * COUNTDOWN_SECONDS;
    round_started = true;

    return true;
}

void game_world::countdownEnd() {
    g_map.createMap(MAP_WIDTH, MAP_HEIGHT, num_users);

    int num = 0;

    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed()) {
            switch (ent->getType()) {
            case TYPE_PLAYER:
                {
                    player *p = dynamic_cast<player *>(ent);
                    if (!p || !p->isNotDestroyed()) {
                        continue;
                    }
                    p->setPlayerNum(num);
                    try {
                        point spawn_pt = g_map.getSpawnPt(p->getPlayerNum());
                        p->respawn(spawn_pt.x, spawn_pt.y);

                        ++num;
                    } catch (int n) {
                        p->setPlayerNum(0xff);
                        // Could not spawn that player
                    }
                }
                break;
            default:
                ent->destroy();
                break;
            }
        }
    }
}

entity **game_world::findEntities(int tx, int ty, entity_type type) {
    static entity *ents[SEARCH_SIZE];
    int i = 0;

    memset(ents, 0, sizeof(ents));
    for (entity *ent : entities) {
        if (ent && ent->isNotDestroyed() && (type == TYPE_NONE ? true : ent->getType() == type) && ent->getTileX() == tx && ent->getTileY() == ty) {
            ents[i] = ent;
            ++i;
        }
    }

    return ents;
}

template<typename T> const T ABS(T x) {
    return x > 0 ? x : -x;
}

ent_movable **game_world::findMovables(float fx, float fy, entity_type type) {
    static ent_movable *ents[SEARCH_SIZE];
    int i = 0;

    memset(ents, 0, sizeof(ents));
    for (entity *ent : entities) {
        ent_movable *mov = dynamic_cast<ent_movable *>(ent);
        if (mov && mov->isNotDestroyed() && (type == TYPE_NONE ? true : ent->getType() == type)) {
            float dx = ABS(mov->getX() - fx);
            float dy = ABS(mov->getY() - fy);
            if (dx < TILE_SIZE / 2 && dy < TILE_SIZE / 2) {
                ents[i] = mov;
                ++i;
            }
        }
    }

    return ents;
}

bool game_world::isWalkable(float fx, float fy, uint8_t flags) {
    int tx = fx / TILE_SIZE + 0.5f;
    int ty = fy / TILE_SIZE + 0.5f;

    tile *t = getMap().getTile(tx, ty);
    if (!t) return false;

    entity **ents = findEntities(tx, ty);
    for (uint8_t i = 0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (!ent) break;
        switch (ent->getType()) {
        case TYPE_ITEM:
            if (flags & WALK_BLOCK_ITEMS) {
                game_item *item = dynamic_cast<game_item *>(ent);
                if (item && item->isAlive()) {
                    return false;
                }
            }
            break;
        case TYPE_BROKEN_WALL:
            return false;
        default:
            break;
        }
    }

    ent_movable **ents_mov = findMovables(fx, fy);
    for (size_t i=0; i<SEARCH_SIZE; ++i) {
        ent_movable *mov = ents_mov[i];
        if (!mov) break;
        switch (mov->getType()) {
        case TYPE_BOMB:
            if (flags & WALK_IGNORE_BOMBS) {
                break;
            } else {
                bomb *b = dynamic_cast<bomb*>(mov);
                if (b && !b->isFlying()) {
                    return false;
                }
            }
            break;
        case TYPE_PLAYER:
            if (flags & WALK_BLOCK_PLAYERS) {
                player *p = dynamic_cast<player *>(mov);
                if (p && p->isAlive() && !p->isFlying()) {
                    return false;
                }
            }
            break;
        default:
            break;
        }
    }

    switch(t->type) {
    case TILE_WALL:
    case TILE_BREAKABLE:
        return false;
    case TILE_SPECIAL:
    {
        tile_entity *special = g_map.getSpecial(t);
        if (special) {
            return special->isWalkable();
        } else {
            return true;
        }
        break;
    }
    default:
        return true;
    }
}
