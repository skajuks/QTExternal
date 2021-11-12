#ifndef ENTITY_H
#define ENTITY_H
#include "Structs.h"
#include <vector>
#include <array>

extern ClientInfo ci[64];  // ci[0] = localplayer
extern Entity e[64];   // ci[0] = localplayer
extern toggleStateData stateData[6];
extern std::vector<VECTOR2>* EnemyPlayers;
extern std::vector<VECTOR2>* TeamPlayers;
extern std::string mapName;
#endif // ENTITY_H
