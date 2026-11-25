# Knight's Ascent

A turn-based combat game in modern C++ (C++23). A knight fights his way through an arena of monsters loaded from a data file. Each monster type has its own combat behaviour.

Built for the Object-Oriented Programming course at the University of Bucharest (Faculty of Mathematics and Computer Science), with an emphasis on clean class design, safe resource management and automated checks.

## Example run

```
Arena: Pestera (5 monstri ramasi)
Inamic curent: Monstru: Grik | HP: 60 | ATK: 15 [Goblin]

Karl loveste cu 65 damage!
Grik: HP ramas 0
Grik a fost invins!
---
...
---
Karl loveste cu 65 damage!
Rattle: HP ramas 0
Rattle isi aduna oasele si se ridica din nou cu 35 HP!
Rattle scartaie oasele si loveste cu 18 damage!
Karl: HP ramas 214
---
...
---
Bonus dragon-slayer activ: +38 damage
Karl loveste cu 103 damage!
Smaug: HP ramas 97
Smaug sufla foc si arde pentru 60 damage!
Karl: HP ramas 129
---
...
Karl a curatat arena de monstri!
```

*(In-game text is in Romanian.)*

## Combat rules

Each round the knight strikes first. If the monster survives, it answers with its special attack. When a monster falls, the next one in the roster steps in.

| Monster  | Behaviour |
|----------|-----------|
| Goblin   | Attacks twice while above half HP, once when wounded |
| Orc      | Enrages below one third of its HP and hits 50% harder |
| Skeleton | Comes back to life at half HP, a limited number of times |
| Dragon   | Adds fire damage to every hit; the knight gets a dragon-slayer bonus against it |

Monsters are defined in [`assets/monsters.txt`](assets/monsters.txt):

```
# type name hp damage [extra]
goblin   Grik   60  15
skeleton Rattle 70  18 1    # revives
dragon   Smaug  200 35 25   # fire bonus
```

## C++ design highlights

- **Polymorphic hierarchy**: an abstract `Monster` base with `Goblin`, `Orc`, `Skeleton` and `Dragon`. Behaviour lives in virtual functions (`specialAttack`, the `tryRevive` hook) and printing uses the non-virtual interface pattern (`operator<<` → `printExtra`).
- **Ownership with smart pointers**: `Battle` owns its monsters through `std::unique_ptr<Monster>`. Deep copies go through a virtual `clone()` (virtual constructor idiom).
- **Copy-and-swap** assignment for `Battle`, with a `noexcept` `swap` and a defaulted move constructor.
- **`dynamic_cast`** used where it is meaningful: the dragon-slayer bonus only applies when the current enemy is a `Dragon`.
- **Custom exception hierarchy** (`GameException` → `InvalidStatsException`, `MonsterFileException`, `DeadKnightException`), thrown from constructors and the file loader and caught in `main`.
- **Templates**: a generic `Inventory<T>` (used for weapons and armour) and a `printHighlight<T>` function template.
- **Data-driven setup**: monsters are parsed from a text file, and malformed lines are reported with their line number.

## Project structure

```
include/   class declarations
src/       implementations
tests/     combat rule tests (CTest)
assets/    monster roster
main.cpp   demo scenario
```

## Build and run

Requires CMake ≥ 3.26 and a C++23 compiler (GCC 12+, Clang 18+ or MSVC 19.4x).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/oop
```

Add `-DUSE_ASAN=ON` at configure time to build with AddressSanitizer and UBSan (Linux and macOS).

## Tests

```sh
ctest --test-dir build --output-on-failure
```

The tests check the combat rules: damage and death at 0 HP, a single counter-attack per round, goblin double attacks, orc rage, skeleton revival, the dragon-slayer bonus, independence of copied battles, loader error handling, and a full arena run with a known outcome.

## Continuous integration

Every push is checked by GitHub Actions:

- builds on Linux (GCC, Clang), macOS (Apple Clang, GCC) and Windows (MSVC, MinGW), with warnings treated as errors
- the test suite
- AddressSanitizer / UBSan and Valgrind runs of the game
- static analysis with cppcheck and clang-tidy

## License

[GNU AGPL-3.0](LICENSE). The CMake and CI setup comes from the course's [oop-template](https://github.com/mcmarius/oop-template).
