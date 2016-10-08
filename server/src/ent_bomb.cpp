#include "ent_bomb.h"
#include "ent_broken_wall.h"
#include "ent_item.h"

#include "player.h"

#include "main.h"

explosion::explosion(game_world *world, bomb *b) : entity(world, TYPE_EXPLOSION) {
    bomb_id = b->getID();

    tx = b->getTileX();
    ty = b->getTileY();

    life_ticks = TICKRATE * 2 / 3;

    int explode_size = b->explode_size;

    piercing = b->piercing;

    trunc_l = false;
    trunc_t = false;
    trunc_r = false;
    trunc_b = false;

    entity **ents = world->findEntities(tx, ty, TYPE_ITEM);
    for (uint8_t i=0; i<SEARCH_SIZE; ++i) {
        if (ents[i]) {
            game_item *item = dynamic_cast<game_item *>(ents[i]);
            item->explode();
        }
    }

    do_send_updates = false;

    tile *t = world->getMap().getTile(tx, ty);
    if (t) {
        switch (t->type) {
        case tile::TILE_WALL:
            return;
        case tile::TILE_BREAKABLE:
        case tile::TILE_ITEM:
            world->addEntity(new broken_wall(world, t));
            t->type = tile::TILE_FLOOR;
            return;
        default:
            break;
        }
    }

    len_l = destroyTiles(-explode_size, 0, &trunc_l);
    len_t = destroyTiles(0, -explode_size, &trunc_t);
    len_r = destroyTiles(explode_size, 0, &trunc_r);
    len_b = destroyTiles(0, explode_size, &trunc_b);

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

        entity **tile_ents = world->findEntities(x, y);
        for (uint8_t j=0; j<SEARCH_SIZE; ++j) {
            entity *ent = tile_ents[j];
            if (! ent) break;
            switch (ent->getType()) {
            case TYPE_BOMB:
                {
                    bomb *b = dynamic_cast<bomb *>(ent);
                    if (bomb_id == b->getID()) {
                        continue;
                    }
                    b->explode();
                    if (!piercing) {
                        *trunc = true;
                        return i;
                    }
                }
                break;
            case TYPE_ITEM:
                {
                    game_item *it = dynamic_cast<game_item *>(ent);
                    it->explode();
                    if (!piercing) {
                        *trunc = true;
                        return i;
                    }
                }
                break;
            default:
                break;
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
    fx = p->getTileX() * TILE_SIZE;
    fy = p->getTileY() * TILE_SIZE;
    fz = 0.f;

    life_ticks = TICKRATE * 4;

    explode_size = p->explosion_size;

    piercing = false;
    kicked = false;

    do_send_updates = false;

    speedx = 0.f;
    speedy = 0.f;
}

void bomb::kick(uint8_t direction) {
    if (kicked || punched) return;

    switch (direction) {
    case 0:
        speedy = -KICK_SPEED;
        break;
    case 1:
        speedy = KICK_SPEED;
        break;
    case 2:
        speedx = -KICK_SPEED;
        break;
    case 3:
        speedx = KICK_SPEED;
        break;
    default:
        return;
    }

    kicked = true;
    do_send_updates = true;
}

void bomb::punch(uint8_t direction) {
    if (kicked || punched) return;

    switch (direction) {
    case 0:
        speedy = -PUNCH_SPEED;
        break;
    case 1:
        speedy = PUNCH_SPEED;
        break;
    case 2:
        speedx = -PUNCH_SPEED;
        break;
    case 3:
        speedx = PUNCH_SPEED;
        break;
    default:
        return;
    }

    speedz = PUNCH_SPEED_Z;

    punched = true;
    do_send_updates = true;
}

void bomb::tick() {
    if (kicked) {
        float to_fx = fx + speedx;
        float to_fy = fy + speedy;

        float check_fx = to_fx;
        float check_fy = to_fy;

        if (speedx > 0) {
            check_fx += TILE_SIZE / 2;
        } else if (speedx < 0) {
            check_fx -= TILE_SIZE / 2;
        } else if (speedy > 0) {
            check_fy += TILE_SIZE / 2;
        } else if (speedy < 0) {
            check_fy -= TILE_SIZE / 2;
        }

        int to_tx = (check_fx / TILE_SIZE) + 0.5f;
        int to_ty = (check_fy / TILE_SIZE) + 0.5f;

        if ((to_tx == getTileX() && to_ty == getTileY()) ||
            world->isWalkable(to_tx, to_ty)) {
            fx = to_fx;
            fy = to_fy;
        } else {
            kicked = false;
            fx = getTileX() * TILE_SIZE;
            fy = getTileY() * TILE_SIZE;
        }
    } else if (punched) {
        fx += speedx;
        fy += speedy;
        fz += speedz;

        speedz -= Z_ACCEL;

        if (speedx > 0 && fx > TILE_SIZE * MAP_WIDTH) {
            fx -= TILE_SIZE * MAP_WIDTH;
        }
        if (speedx < 0 && fx < 0) {
            fx += TILE_SIZE * MAP_WIDTH;
        }
        if (speedy > 0 && fy > TILE_SIZE * MAP_HEIGHT) {
            fy -= TILE_SIZE * MAP_HEIGHT;
        }
        if (speedy < 0 && fy < 0) {
            fy += TILE_SIZE * MAP_HEIGHT;
        }

        if (fz <= 0) {
            if (world->isWalkable(getTileX(), getTileY(), true)) {
                fx = getTileX() * TILE_SIZE;
                fy = getTileY() * TILE_SIZE;
                fz = 0.f;
                punched = false;
            } else {
                speedx *= XY_BOUNCE;
                speedy *= XY_BOUNCE;
                speedz *= -Z_BOUNCE;
            }
        }
    }

    if (life_ticks == 0) {
        explode();
    }
    --life_ticks;
}

void explosion::checkPlayers(int dx, int dy) {
    int x = tx;
    int y = ty;
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

        entity **ents = world->findEntities(x, y, TYPE_PLAYER);
        for (uint8_t i=0; i < SEARCH_SIZE; ++i) {
            if (!ents[i]) break;
            player *p = dynamic_cast<player*>(ents[i]);
            p->kill();
        }
    }
}

void explosion::tick() {
    entity **ents = world->findEntities(tx, ty);
    for (uint8_t i=0; i < SEARCH_SIZE; ++i) {
        entity *ent = ents[i];
        if (!ent) break;
        switch (ent->getType()) {
        case TYPE_PLAYER:
            dynamic_cast<player*>(ent)->kill();
            break;
        }
    }

    checkPlayers(-len_l, 0);
    checkPlayers(0, -len_t);
    checkPlayers(len_r, 0);
    checkPlayers(0, len_b);

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
    packet.writeShort(14);
    packet.writeInt(fx);
    packet.writeInt(fy);
    packet.writeInt(fz);
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
