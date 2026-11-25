#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "armor.h"
#include "battle.h"
#include "dragon.h"
#include "exceptions.h"
#include "goblin.h"
#include "knight.h"
#include "monster_loader.h"
#include "orc.h"
#include "skeleton.h"
#include "stats.h"
#include "weapon.h"

namespace {

class Checker {
private:
    int failures = 0;

public:
    void check(bool condition, const std::string& what) {
        if (!condition) {
            ++failures;
            std::cerr << "  FAIL: " << what << "\n";
        }
    }

    [[nodiscard]] bool passed() const { return failures == 0; }
};

class SilenceCout {
private:
    std::ostringstream sink;
    std::streambuf* old;

public:
    SilenceCout() : old(std::cout.rdbuf(sink.rdbuf())) {}
    ~SilenceCout() { std::cout.rdbuf(old); }
    SilenceCout(const SilenceCout&) = delete;
    SilenceCout& operator=(const SilenceCout&) = delete;
};

Knight makeKnight(int hp, int attack) {
    return {"Test", Stats(hp, attack), Weapon("Nimic", 0), Armor("Nimic", 0)};
}

std::vector<std::unique_ptr<Monster>> single(std::unique_ptr<Monster> m) {
    std::vector<std::unique_ptr<Monster>> roster;
    roster.push_back(std::move(m));
    return roster;
}

std::filesystem::path writeTempFile(const std::string& name, const std::string& content) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream out(path);
    out << content;
    return path;
}

template <typename ExceptionType>
bool throwsException(const std::function<void()>& action) {
    try {
        action();
    } catch (const ExceptionType&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

void monsterDiesAtZeroHp(Checker& c) {
    Goblin g("Grik", 60, 10);
    g.receiveDamage(65);
    c.check(!g.isAlive(), "goblinul ar trebui sa moara dupa 65 damage la 60 HP");
    c.check(g.getHp() == 0, "HP-ul nu ar trebui sa scada sub 0");
}

void knightDamagesMonsterAndMonsterRespondsOnce(Checker& c) {
    Knight hero = makeKnight(100, 50);
    Battle arena("Test", single(std::make_unique<Orc>("Brutus", 120, 10)));

    hero.fight(arena);
    c.check(hero.getTotalHP() == 90, "dupa runda 1 cavalerul ar trebui sa aiba 90 HP");
    c.check(!arena.finished(), "orcul ar trebui sa supravietuiasca primei lovituri");

    hero.fight(arena);
    c.check(hero.getTotalHP() == 75, "orcul infuriat ar trebui sa dea 15 damage");

    hero.fight(arena);
    c.check(hero.getTotalHP() == 75, "un monstru invins nu ar trebui sa mai atace");
    c.check(arena.finished(), "arena ar trebui sa fie goala dupa ce orcul moare");
}

void goblinAttacksTwiceOnlyAboveHalfHp(Checker& c) {
    Knight hero = makeKnight(100, 0);
    Goblin g("Snik", 60, 10);

    g.specialAttack(hero);
    c.check(hero.getTotalHP() == 80, "goblinul sanatos ar trebui sa atace de 2 ori");

    g.receiveDamage(30);
    g.specialAttack(hero);
    c.check(hero.getTotalHP() == 70, "goblinul ranit ar trebui sa atace o singura data");
}

void skeletonRevivesExactlyOnce(Checker& c) {
    Skeleton s("Rattle", 70, 5, 1);
    s.receiveDamage(70);
    c.check(s.isAlive(), "scheletul ar trebui sa invie prima data");
    c.check(s.getHp() == 35, "scheletul ar trebui sa invie cu jumatate din viata");

    s.receiveDamage(35);
    c.check(!s.isAlive(), "scheletul fara reinvieri ar trebui sa ramana mort");
}

void dragonSlayerBonusOnlyAgainstDragons(Checker& c) {
    const Battle vsDragon("Test", single(std::make_unique<Dragon>("Smaug", 200, 35, 20)));
    c.check(vsDragon.dragonSlayerBonus(40) == 65, "bonusul contra dragonului: 40 + 40/2 + 20/4 = 65");

    const Battle vsGoblin("Test", single(std::make_unique<Goblin>("Grik", 60, 10)));
    c.check(vsGoblin.dragonSlayerBonus(40) == 40, "fara bonus contra altor monstri");
}

void fullArenaMatchesExpectedOutcome(Checker& c) {
    const auto path = writeTempFile("knights_ascent_arena.txt",
                                    "goblin Grik 60 15\n"
                                    "skeleton Rattle 70 18 1\n"
                                    "orc Brutus 120 25\n"
                                    "dragon Smaug 200 35 25\n"
                                    "goblin Snik 40 12\n");
    Battle arena("Pestera", MonsterLoader::loadFromFile(path.string()));
    Knight karl("Karl", Stats(200, 25), Weapon("Sabie de foc", 40), Armor("Aur", 50));

    int rounds = 0;
    while (!arena.finished() && karl.isAlive() && rounds < 100) {
        karl.fight(arena);
        ++rounds;
    }
    std::filesystem::remove(path);

    c.check(arena.finished(), "Karl ar trebui sa curete arena");
    c.check(rounds == 9, "lupta ar trebui sa dureze 9 runde, a durat " + std::to_string(rounds));
    c.check(karl.getTotalHP() == 129, "Karl ar trebui sa termine cu 129 HP, are " + std::to_string(karl.getTotalHP()));
}

void battleCopyIsIndependent(Checker& c) {
    const Battle original("Test", single(std::make_unique<Goblin>("Grik", 60, 10)));
    Battle copy = original;
    Knight hero = makeKnight(100, 100);
    hero.fight(copy);
    c.check(copy.finished(), "copia ar trebui sa fie terminata");
    c.check(!original.finished(), "originalul nu ar trebui afectat de lupta din copie");
}

void loaderRejectsBadFiles(Checker& c) {
    c.check(throwsException<MonsterFileException>([] {
              (void)MonsterLoader::loadFromFile("nu_exista.txt");
          }),
          "fisier lipsa -> MonsterFileException");

    const auto unknown = writeTempFile("knights_ascent_unknown.txt", "troll Bob 10 10\n");
    c.check(throwsException<MonsterFileException>([&] {
              (void)MonsterLoader::loadFromFile(unknown.string());
          }),
          "tip de monstru necunoscut -> MonsterFileException");
    std::filesystem::remove(unknown);

    const auto empty = writeTempFile("knights_ascent_empty.txt", "# doar comentarii\n");
    c.check(throwsException<MonsterFileException>([&] {
              (void)MonsterLoader::loadFromFile(empty.string());
          }),
          "fisier fara monstri -> MonsterFileException");
    std::filesystem::remove(empty);
}

void invalidStateThrows(Checker& c) {
    c.check(throwsException<InvalidStatsException>([] { (void)Stats(-1, 5); }),
          "HP negativ -> InvalidStatsException");

    c.check(throwsException<DeadKnightException>([] {
              Knight ghost = makeKnight(0, 5);
              Battle arena("Test", single(std::make_unique<Goblin>("Grik", 60, 10)));
              ghost.fight(arena);
          }),
          "un cavaler mort nu poate lupta -> DeadKnightException");
}

}

int main() {
    const std::vector<std::pair<std::string, void (*)(Checker&)>> tests = {
        {"monsterDiesAtZeroHp", monsterDiesAtZeroHp},
        {"knightDamagesMonsterAndMonsterRespondsOnce", knightDamagesMonsterAndMonsterRespondsOnce},
        {"goblinAttacksTwiceOnlyAboveHalfHp", goblinAttacksTwiceOnlyAboveHalfHp},
        {"skeletonRevivesExactlyOnce", skeletonRevivesExactlyOnce},
        {"dragonSlayerBonusOnlyAgainstDragons", dragonSlayerBonusOnlyAgainstDragons},
        {"fullArenaMatchesExpectedOutcome", fullArenaMatchesExpectedOutcome},
        {"battleCopyIsIndependent", battleCopyIsIndependent},
        {"loaderRejectsBadFiles", loaderRejectsBadFiles},
        {"invalidStateThrows", invalidStateThrows},
    };

    bool allPassed = true;
    for (const auto& [name, test] : tests) {
        Checker checker;
        {
            const SilenceCout silence;
            test(checker);
        }
        allPassed = allPassed && checker.passed();
        std::cout << (checker.passed() ? "[ OK ]   " : "[FAIL]   ") << name << "\n";
    }

    std::cout << "\n" << (allPassed ? "Toate testele au trecut." : "Unele teste au picat.") << "\n";
    return allPassed ? 0 : 1;
}
