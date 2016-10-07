#include "ent_bomb.h"
#include "ent_broken_wall.h"
#include "ent_item.h"

#include "player.h"

#include "main.h"

explosion::explosion(game_world *world, bomb *b) : entity(world, TYPE_EXPLOSION) {
    bomb_id = b->getID();

    tx = b->tx;
    ty = b->ty;

    life_ticks = TICKRATE;

    int explode_size = b->explode_size;

    piercing = b->piercing;

    trunc_l = false;
    trunc_t = false;
    trunc_r = false;
    trunc_b = false;

    len_l = destroyTiles(-explode_size, 0, &trunc_l);
    len_t = destroyTiles(0, -explode_size, &trunc_t);
    len_r = destroyTiles(explode_size, 0, &trunc_r);
    len_b = destroyTiles(0, explode_size, &trunc_b);

    do_send_updates = false;
}

uint8_t explosion::destroyTiles(int dx, int dy, bool *trunc) {
    int x = tx;
    int y = ty;
    uint8_t i = 0;
    while(true) {
        if (dx < 0) {
            --x;
            ++dx;
        } else if (dx > 0) {
            ++x;
            --dx;
        } else if (dy < 0) {
            --y;
            ++dy;
        } else if (dy > 0) {
            ++y;
            --dy;
        } else {
            break;
        }

        for (entity *ent : world->getEntities()) {
            if (ent->getTileX() == x && ent->getTileY() == y) {
                if (ent && ent->getType() == TYPE_BOMB) {
                    bomb *b = (bomb *)ent;
                    if (bomb_id == b->getID()) {
                        continue;
                    }
                    b->explode();
                    if (!piercing) {
                        *trunc = true;
                        return i;
                    }
                } else if (ent && ent->getType() == TYPE_ITEM) {
                    game_item *it = (game_item *)ent;
                    it->explode();
                    if (!piercing) {
                        *trunc = true;
                        return i;
                    }
                }
            }
        }

        tile *t = world->getMap().getTile(x, y);
        if (!t) {
            break;
        }
        switch (t->type) {
        case tile::TILE_ITEM:
        case tile::TILE_BREAKABLE:
            world->addEntity(new broken_wall(world, t));
            t->type = tile::TILE_FLOOR;
            if (piercing) {
                break;
            } else {
                *trunc = true;
            }
        case tile::TILE_WALL:
            return i;
        default:
            break;
        }
        ++i;
    }
    return i;
}

bomb::bomb(game_world *world, player *p) : entity(world, TYPE_BOMB), p(p) {
    tx = p->getTileX();
    ty = p->getTileY();

    life_ticks = TICKRATE * 4;

    explode_size = p->explosion_size;

    piercing = false;

    do_send_updates = false;
}

void bomb::tick() {
    if (life_ticks == 0) {
        explode();
    }
    --life_ticks;
}

void explosion::tick() {
    // TODO check players in area of explosion
    if (life_ticks == 0) {
        destroy();
    }
    --life_ticks;
}

void bomb::explode() {
    if (exploded) return;

    ++p->num_bombs;

    exploded = true;
    destroy();
    world->addEntity(new explosion(world, this));
}

void bomb::writeEntity(packet_ext &packet) {
    packet.writeShort(4);
    packet.writeChar(tx);
    packet.writeChar(ty);
    packet.writeShort(p->getID());
}

void explosion::writeEntity(packet_ext &packet) {
    packet.writeShort(9);
    packet.writeChar(tx);
    packet.writeChar(ty);
    packet.writeChar(len_l);
    packet.writeChar(len_t);
    packet.writeChar(len_r);
    packet.writeChar(len_b);

    uint8_t trunc = 0;
    trunc |= (1 << 0) * trunc_l;
    trunc |= (1 << 1) * trunc_t;
    trunc |= (1 << 2) * trunc_r;
    trunc |= (1 << 3) * trunc_b;

    packet.writeChar(trunc);
    packet.writeShort(bomb_id);
}
