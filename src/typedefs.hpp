#include <SDL2/SDL_stdinc.h>

#define MAX_POLYS 10
#define MAX_VERTS 8

#define SHOULD_RASTERIZE 1 // 1 is on and 0 if off
#define RASTER_RESOLUTION 1 // decrease for better resolution, increase for performance
#define RASTER_NUM_VERTS 4

typedef struct Vec2 {
    float x, y;
} Vec2;
 
typedef struct {
    Vec2 p1, p2;
} LineSeg;
 
typedef struct {
    Vec2 vert[MAX_VERTS];
    int vertCnt;
    float height;
    float curDist;
} Polygon;
 
typedef struct {
    Vec2 vert[4];
    float distFromCamera;
    int planeIdInPoly;
} ScreenSpacePoly;

typedef struct{
    float camAngle;
    float stepWave;
    Vec2 camPos;
    Vec2 oldCamPos;
} Camera;

typedef struct {
    Uint8 R, G, B;
} Color;