#include "monster.hpp"

#include "draw.hpp"
#include "hero.hpp"

Monster::Monster() {
    for(int i = 0; i < CONDITION_COUNT; ++i) {
        conditionTimers[static_cast<Condition>(i)] = 0;
    }
}

bool Monster::operator==(const Monster& other) const {
    return name == other.name && pos == other.pos;
}

bool Monster::affectedBy(Condition condition) const {
    auto it = conditionTimers.find(condition);
    return it != conditionTimers.end() && it->second != 0;
}

void Monster::takeTurn(GameState& state) {
    if (portalTimer > 0) { // Stil spawning
      portalTimer--;
      // Once the monster has spawned
      if (portalTimer == 0) {
         state.setTile(pos, Tile::MONSTER);
      }
    } else if (affectedBy(Condition::SLEEPING)) { // Sleeping
        conditionTimers[Condition::SLEEPING]--;
        if (!affectedBy(Condition::SLEEPING)) {
            auto& text = CONDITION_END.at(Condition::SLEEPING);
            state.addMessage(text.first + name + text.second, MessageType::SPELL);
        }
    } else {
        if (timer == 0) { // Ready to act
            takeAction(state);
            decreaseConditionTimers(state);
            timer = wait;
        }
        timer -= 1;
    }
}

void Monster::decreaseConditionTimers(GameState& state) {
    for (auto& [condition, timer]: conditionTimers) {
        if (timer > 0) {
            timer--;
            if (timer == 0) {
                const auto& text = CONDITION_END.at(condition);
                state.addMessage(text.first + name + text.second, MessageType::SPELL);
            }
        }
    }
}

void Monster::takeAction(GameState& state) {
    if (symbol == '*') { // Master Summoner boss will summon a monster
        if (state.monsterList.size() < MAX_MONSTERS) {
            Position l = Utils::randomMapPosWithCondition(
                [&state](const auto& pos){return state.tileAt(pos) == Tile::BLANK;}
            );
            state.addSpecifiedMonster(l.x, l.y, Utils::randGen->getInt(0, 12), true);
        }
    } else {
        state.tileAt(pos) = Tile::BLANK;

        ActionDecision decision = decideOnAction(state);
        if (std::holds_alternative<RangedAttack>(decision)) { // Monster decided on ranged attack
            rangedAttackHero(state);
        } else { // Monster decided to move (or attack adjacent target)
            performMove(state, std::get<MoveTo>(decision).pos);
        }

        if (health > 0) {
            state.tileAt(pos) = Tile::MONSTER;
        }
    }
}

Monster::ActionDecision Monster::decideOnAction(const GameState& state) {
    Hero& hero = *(state.hero);

    Position desiredPos;
    if (affectedBy(Condition::HALTED)) {
        // Halted monster can still attack adjacent hero
        if (Utils::dist(hero.pos, pos) == 1.0) {
            desiredPos = {
                Utils::signum(hero.pos.x - pos.x),
                Utils::signum(hero.pos.y - pos.y)
            };
        }
    } else if (affectedBy(Condition::FLEEING) && state.isInFov(pos)) {
        // Fleeing monster will run from hero it can see
        desiredPos = {
            Utils::signum(pos.x - hero.pos.x),
            Utils::signum(pos.y - hero.pos.y)
        };
    } else if (affectedBy(Condition::RAGED) || affectedBy(Condition::ALLIED)) {
        // Enraged or allied monsters will consider attacking other monsters
        float heroDist = Utils::dist(hero.pos, pos);
        float nearestMonsterDist = MAP_WIDTH + MAP_HEIGHT + 2;
        const Monster* nearestMonster;

        state.map.model->computeFov(pos.x, pos.y);
        for (const Monster& otherMon : state.monsterList) {
            if ((&otherMon != this) && state.map.model->isInFov(otherMon.pos.x, otherMon.pos.y)) {
                if (float tempdist = Utils::dist(pos, otherMon.pos); tempdist < nearestMonsterDist) {
                    nearestMonsterDist = tempdist;
                    nearestMonster = &otherMon;
                }
            }
        }
        state.map.model->computeFov(hero.pos.x, hero.pos.y);

        // Should this monster attack the nearest other monster or the hero?
        int targetDist = -1;
        Position targetPos;
        if (affectedBy(Condition::ALLIED) || nearestMonsterDist < heroDist) {
            // Allied monster or enraged monster closer to another monster
            targetDist = nearestMonsterDist;
            targetPos = nearestMonster->pos;
        } else {
            targetDist = heroDist;
            targetPos = hero.pos;
        }

        // Blind monster not next to target will randomly stumble around
        if (affectedBy(Condition::BLINDED) && targetDist != 1.0) {
            desiredPos = {
                Utils::randGen->getInt(-1, 1),
                Utils::randGen->getInt(-1, 1)
            };
        } else {
            desiredPos = {
                Utils::signum(targetPos.x - pos.x),
                Utils::signum(targetPos.y - pos.y)
            };
        }
    } else if (angry && !hero.dead && (!affectedBy(Condition::BLINDED) || hero.isAdjacent(pos))) {
        // Aggro'd monster

        if (
            ranged && // Ranged monster
            !hero.isAdjacent(pos) && // not directly next to hero
            Utils::dist(pos, hero.pos) <= range && // but hero is within range
            state.map.model->isInFov(pos.x, pos.y) // and we can see them
        ) {
            return RangedAttack{};
        }

        desiredPos = {
            Utils::signum(hero.pos.x - pos.x),
            Utils::signum(hero.pos.y - pos.y)
        };
    } else { // Non-aggro'd monster

        // Will aggro if
        if (state.map.model->isInFov(pos.x, pos.y) && // we can see the hero
            (!affectedBy(Condition::BLINDED) || hero.isAdjacent(pos)) && // and not blind (or is adjacent)
            (symbol != '@' || health != maxhealth)) { // (noble hero boss will only aggro if hit first)
            angry = true;
        }
        desiredPos = {
            Utils::randGen->getInt(-1, 1),
            Utils::randGen->getInt(-1, 1)
        };
    }

    // Monster can only step orthogonally so limit to one axis to move along
    if (desiredPos.x != 0 && desiredPos.y != 0) {
        return Utils::randGen->getInt(0, 1) == 0
            ? MoveTo(Position(0, desiredPos.y))
            : MoveTo(Position(desiredPos.x, 0));
    }
    return MoveTo(desiredPos);
}

void Monster::rangedAttackHero(GameState& state) {
    Hero& hero = *(state.hero);
    Draw::rangedAttack(hero.pos, pos);
    int damage = getDamageDealt();
    if (hero.shieldTimer == 0) {
        hero.health -= damage;
        char buffer[20];
        sprintf(buffer, "%d", damage);
        state.addMessage("The " + name + " " + rangedName + " at the hero for " + buffer + " damage", MessageType::NORMAL);
        if (hero.health <= 0) {
            hero.die();
        } else if (hero.meditationTimer > 0) {
            hero.meditationTimer = 0;
            state.addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
        }
    } else {
        hero.shieldTimer -= damage;
        if (hero.shieldTimer <= 0) {
        hero.shieldTimer = 0;
        state.addMessage("The shield is shattered by the " + name + "'s attack!", MessageType::SPELL);
        } else {
        state.addMessage("The " + name + "'s attack is deflected by the shield", MessageType::SPELL);
        }
    }
    if (maimed) {
        state.addMessage("The "+name+" suffers from the exertion!", MessageType::NORMAL);
        state.hitMonster(pos, damage);
    }
}

void Monster::performMove(GameState& state, Position diff) {
    Hero& hero = *(state.hero);
    Position diffPos = pos.offset(diff);
    Tile& diffTile = state.tileAt(diffPos);

    if (diffPos == hero.pos && !affectedBy(Condition::ALLIED)) {
        if (hero.dead) {
            hero.health -= damage;
            if (symbol != '@') {
                state.addMessage("The " + name + " savages the hero's corpse", MessageType::NORMAL);
            }
        } else {
            int damage = getDamageDealt();
            hero.health -= damage;
            char buffer[20];
            sprintf(buffer, "%d", damage);
            state.addMessage("The " + name + " hits the hero for " + buffer + " damage", MessageType::NORMAL);
            if (hero.health <= 0) {
                hero.die();
            } else if (hero.meditationTimer > 0) {
                hero.meditationTimer = 0;
                state.addMessage("The hero's meditation is interrupted!", MessageType::SPELL);
            } else if (hero.target != this && hero.target != nullptr) {
                if (Utils::dist(hero.pos, pos) < Utils::dist(hero.pos, hero.target->pos)) {
                    hero.target = this;
                }
            }
            if (maimed) {
                state.addMessage("The "+name+" suffers from the exertion!", MessageType::NORMAL);
                state.hitMonster(pos, damage);
            }
        }
    } else if (diffTile == Tile::MONSTER && (diff.x != 0 || diff.y != 0)) {
        if (Monster* otherMonster = state.findMonster(diffPos); otherMonster != nullptr) {
            if (affectedBy(Condition::RAGED) || affectedBy(Condition::ALLIED)) {
                char buffer[20];
                sprintf(buffer, "%d", damage);
                state.addMessage("The " + name + " hits the " + otherMonster->name + " for " +buffer + " damage", MessageType::NORMAL);
                state.hitMonster(diffPos, damage);
            } else {
                state.addMessage("The " + name + " bumps into the " + otherMonster->name, MessageType::NORMAL);
            }
        }
    } else if (diffTile == Tile::PLAYER) {
        std::swap(state.player.pos, pos);
        state.addMessage("The " + name + " passes through you", MessageType::NORMAL);
        state.tileAt(state.player.pos) = Tile::PLAYER;
    } else if (diffTile == Tile::TRAP) {
        state.addMessage("The " + name+ " falls into the trap!", MessageType::NORMAL);
        pos = diffPos;
        diffTile = Tile::MONSTER;
        state.hitMonster(pos, 4);
    } else if (diffTile == Tile::ILLUSION) {
        diffTile = Tile::BLANK;
        state.illusion = {1, -1};
        state.addMessage("The "+name+" disrupts the illusion", MessageType::SPELL);
    } else if (diffTile == Tile::BLANK) {
        pos = diffPos;
    }
}