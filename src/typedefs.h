#define MAX_POLYS 10
#define MAX_VERTS 8

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