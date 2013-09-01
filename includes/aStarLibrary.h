#include "hardheaded.h"

void InitializePathfinder();

int FindPath (int pathfinderID,int startingX, int startingY,
			  int targetX, int targetY);
int ReadPath(int pathfinderID,int currentX,int currentY, int pixelsPerFrame);
void EndPathfinder();

#define mapWidth  32
#define mapHeight 24
#define tileSize  32
#define enemyTileHeight  3 // FIXME chan change by enemy?
#define numberPeople ENEMY_COUNT
#define maxPathLength ((mapWidth+mapHeight)*4)
#define notfinished 0
#define notStarted 0// path-related constants
#define found 1
#define nonexistent 2
#define walkable  0
#define unwalkable 1// walkability array constants

extern int walkability[mapWidth][mapHeight];
extern int pathStatus[numberPeople+1];
extern int xPath[numberPeople+1];
extern int yPath[numberPeople+1];
extern int pathLength[numberPeople+1];     //stores length of the found path for critter
