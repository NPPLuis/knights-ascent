#include "../include/goblin.h"
#include "../include/knight.h"
#include <iostream>

Goblin::Goblin(const std::string& name_val, int hp_val, int damage_val)
    : Monster(name_val, hp_val, damage_val) {}

std::unique_ptr<Monster> Goblin::clone() const {
    return std::make_unique<Goblin>(*this);
}

void Goblin::specialAttack(Knight& target) const {
    const int hit = getDamage();
    if (getHp() * 2 > getMaxHp()) {
        std::cout << getName() << " ataca rapid de 2 ori cu " << hit << " damage!\n";
        target.receiveDamage(hit);
        target.receiveDamage(hit);
    } else {
        std::cout << getName() << " e obosit si ataca o data cu " << hit << " damage!\n";
        target.receiveDamage(hit);
    }
}

void Goblin::printExtra(std::ostream& os) const {
    os << " [Goblin]";
}
