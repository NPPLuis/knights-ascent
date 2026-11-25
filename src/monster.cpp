#include "../include/monster.h"
#include "../include/knight.h"
#include <iostream>
#include <utility>

Monster::Monster(std::string name_val, int hp_val, int damage_val)
    : name(std::move(name_val)), hp(hp_val), maxHp(hp_val), damage(damage_val) {}

void Monster::attackKnight(Knight& target) const {
    std::cout << name << " l-a atacat pe " << target.getName() << " cu " << damage << " damage!\n";
    target.receiveDamage(damage);
}

void Monster::receiveDamage(int amount) {
    hp -= amount;
    if (hp < 0) hp = 0;
    std::cout << name << ": HP ramas " << hp << "\n";
    if (hp == 0 && !tryRevive()) {
        std::cout << name << " a fost invins!\n";
    }
}

bool Monster::tryRevive() {
    return false;
}

void Monster::restoreHp(int amount) {
    hp = amount > maxHp ? maxHp : amount;
}

int Monster::getHp() const { return hp; }

int Monster::getMaxHp() const { return maxHp; }

const std::string& Monster::getName() const { return name; }

int Monster::getDamage() const { return damage; }

bool Monster::isAlive() const { return hp > 0; }

void Monster::printExtra(std::ostream& os) const {
    (void)os;
}

std::ostream& operator<<(std::ostream& os, const Monster& m) {
    os << "Monstru: " << m.name << " | HP: " << m.hp << " | ATK: " << m.damage;
    m.printExtra(os);
    return os;
}
