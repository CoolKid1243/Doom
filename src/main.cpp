#include "typedefs.hpp"

#include <SDL2/SDL.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>

#undef main

#define RES_DIV 3
#define screenW 1152 / RES_DIV
#define screenH 758 / RES_DIV

#define MOV_SPEED 100
#define ROT_SPEED 3
#define WWAVE_MAG 15

#define POL_RES 1.025 // point on line check resolution

// Global variables
SDL_Renderer* renderer;

Camera cam;
Polygon polys[MAX_POLYS];

int screenSpaceVisiblePlanes;
ScreenSpacePoly screenSpacePolys[MAX_POLYS][MAX_VERTS];

void Init();
void CameraTranslate(double deltaTime);
Color GetColorByDistance(float dist);
void Rasterize();
void ClearRasterBuffer();
void Render();
void UpdateScreen();
int ShouldQuit(SDL_Event event);

// Math
float DotPoints(float x1, float y1, float x2, float y2);
float Dot(Vec2 pointA, Vec2 pointB);
Vec2 Normalize(Vec2 vec);
Vec2 VecMinus(Vec2 v1, Vec2 v2);
Vec2 VecPlus(Vec2 v1, Vec2 v2);
Vec2 VecMulF(Vec2 v1, float val);
float Len(Vec2 pointA, Vec2 pointB);

// Physics
int LineCircleCollision(LineSeg line, Vec2 circleCenter, float circleRadius);
Vec2 ResolveCollision(Vec2 lastPosition, Vec2 currentPosition, LineSeg lineOfCollision, float deltaTime);
void CollisionDetection(float deltaTime);

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* mainWin = SDL_CreateWindow(
        "knock-off doom",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1152, 758,
        SDL_WINDOW_SHOWN
    );
 
    renderer = SDL_CreateRenderer(mainWin, 0, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetLogicalSize(renderer, screenW, screenH);

    Init();

    int loop = 1;
    SDL_Event event;
    double deltaTime = 0.016;

    while (loop) {
        double start = SDL_GetTicks();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // window clear color
        SDL_RenderClear(renderer);

        SDL_PollEvent(&event);

        cam.oldCamPos = cam.camPos;
        CameraTranslate(deltaTime);
        CollisionDetection(deltaTime);
        Render();
        UpdateScreen();

        double end = SDL_GetTicks();
        deltaTime = (end - start) / 1000.0;

        while (SDL_PollEvent(&event)) if (ShouldQuit(event)) loop = 0;
    }
 
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWin);
    SDL_Quit();
    return 0;
}

int ShouldQuit(SDL_Event event) {
    if(event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) return 1;
    return 0;
}

void PutPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    if (x > screenW || y > screenH) return;
    if (x < 0 || y < 0) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}

void DrawLine(int x0, int y0, int x1, int y1) {
    int dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 > y0) ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        PutPixel(x0, y0, 255, 0, 0);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}


float Cross2dPoints(float x1, float y1, float x2, float y2) {
    return x1 * y2 - y1 * x2;
}

Vec2 Intersection(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    Vec2 p;

    p.x = Cross2dPoints(x1, y1, x2, y2);
    p.y = Cross2dPoints(x3, y3, x4, y4);
    float det = Cross2dPoints(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    p.x = Cross2dPoints(p.x, x1 - x2, p.y, x3 - x4) / det;
    p.y = Cross2dPoints(p.x, y1 - y2, p.y, y3 - y4) / det;
    
    return p;
}

int IsFrontFace(Vec2 Camera, Vec2 pointA, Vec2 pointB) {
    const int RIGHT = 1, LEFT = -1, ZERO = 0;
    pointA.x -= Camera.x;
    pointA.y -= Camera.y;
    pointB.x -= Camera.x;
    pointB.y -= Camera.y;
    int cross_product = pointA.x * pointB.y - pointA.y * pointB.x;
    
    if (cross_product > 0) return RIGHT;
    if (cross_product < 0) return LEFT;
    
    return ZERO;
}

void CameraTranslate(double deltaTime) {
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
 
    if (keyState[SDL_SCANCODE_W]) {
        cam.camPos.x += MOV_SPEED * cos(cam.camAngle) * deltaTime;
        cam.camPos.y += MOV_SPEED * sin(cam.camAngle) * deltaTime;
        cam.stepWave += 3 * deltaTime;
    } else if (keyState[SDL_SCANCODE_S]) {
        cam.camPos.x -= MOV_SPEED * cos(cam.camAngle) * deltaTime;
        cam.camPos.y -= MOV_SPEED * sin(cam.camAngle) * deltaTime;
        cam.stepWave += 3 * deltaTime;
    }

    if (cam.stepWave > M_PI*2) cam.stepWave = 0;
 
    if (keyState[SDL_SCANCODE_A]) {
        cam.camAngle -= ROT_SPEED * deltaTime;
    } else if (keyState[SDL_SCANCODE_D]) {
        cam.camAngle += ROT_SPEED * deltaTime;
    }
}

// nvert = vertices count
// vertx = all x vertices coordinates
// verty = all y vertices coordinates
// testx & testy = point to test if inside the polygon
int PointInPoly(int nvert, float *vertx, float *verty, float testx, float testy) {
    int i, j, isPointInside = 0;
 
    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        int isSameCoordinates = 0;
        
        if ((verty[i]>testy) == (verty[j]>testy)) isSameCoordinates = 1;
 
        if (isSameCoordinates == 0 && (testx < (vertx[j]-vertx[i]) * (testy - verty[i]) / (verty[j]-verty[i]) + vertx[i])) {
            isPointInside = !isPointInside;
        }
    }
    
    return isPointInside;
}

Color GetColorByDistance(float dist) {
    float pixelShader = (0x55 / dist);
    if (pixelShader > 1) pixelShader = 1.0;
    else if (pixelShader < 0) pixelShader = 0.1;
    
    Color clr;
    clr.R = 0x00;
    clr.G = 0xFF * pixelShader;
    clr.B = 0x00;
 
    return clr;
}

void RenderSky() {
    int maxy = static_cast<float>(screenH) / 2 + (WWAVE_MAG * sinf(cam.stepWave));
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = screenW;
    rect.h = maxy;
    
    Color clr;
    clr.R = 77;
    clr.G = 181;
    clr.B = 255;
    SDL_SetRenderDrawColor(renderer, clr.R, clr.G, clr.B, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void RenderGround() {
    float waveVal = WWAVE_MAG * sin(cam.stepWave);
    int starty = static_cast<float>(screenH) / 2 + waveVal;
    
    for (int y = starty; y < screenH; y++) {
        Color clr;
        clr.R = y / 2;
        clr.G = y / 2;
        clr.B = y / 2;
        SDL_SetRenderDrawColor(renderer, clr.R, clr.G, clr.B, 255);
        SDL_RenderDrawLine(renderer, 0, y, screenW, y);
    }
}

Vec2 ClosestPointOnLine(LineSeg line, Vec2 point) {
    float lineLen = Len(line.p1, line.p2);
    float dot =
        (((point.x - line.p1.x) * (line.p2.x - line.p1.x)) +
        ((point.y - line.p1.y) * (line.p2.y - line.p1.y))) /
        (lineLen*lineLen);
 
    if (dot > 1)
        dot = 1;
    else if (dot < 0)
        dot = 0;
       
    Vec2 closestPoint;
    closestPoint.x = line.p1.x + (dot * (line.p2.x - line.p1.x));
    closestPoint.y = line.p1.y + (dot * (line.p2.y - line.p1.y));
 
    return closestPoint;
}
 
int IsPointOnLine(LineSeg line, Vec2 point) {
    float lineLen = Len(line.p1, line.p2);
    float pointDist1 = Len(point, line.p1);
    float pointDist2 = Len(point, line.p2);
    float resolution = POL_RES;
    float lineLenMarginHi = lineLen + resolution;
    float lineLenMarginLo = lineLen - resolution;
    float distFromLineEnds = pointDist1 + pointDist2;
 
    if (distFromLineEnds >= lineLenMarginLo &&
        distFromLineEnds <= lineLenMarginHi)
        return 1;
       
    return 0;
}

void ClearRasterBuffer() {
    for (int polyIdx = 0; polyIdx < MAX_POLYS; polyIdx++) {
        for (int i = 0; i < polys[polyIdx].vertCnt; i++) {          
            for (int vn = 0; vn < RASTER_NUM_VERTS; vn++) {
                screenSpacePolys[polyIdx][i].vert[vn].x = 0;
                screenSpacePolys[polyIdx][i].vert[vn].y = 0;
            }
        }
    }
}

void Rasterize() {
    RenderSky();
    RenderGround();

    float vx[4];
    float vy[4];
    
    Uint8 pixelBuff[screenH][screenW];
    memset(pixelBuff, 0, sizeof(pixelBuff));
    
    for (int polyIdx = screenSpaceVisiblePlanes - 1; polyIdx >= 0; polyIdx--) {
        for (int nextv = 0; nextv < RASTER_NUM_VERTS; nextv++) {
            int planeId = screenSpacePolys[polyIdx]->planeIdInPoly;
            
            vx[nextv] = screenSpacePolys[polyIdx][planeId].vert[nextv].x;
            vy[nextv] = screenSpacePolys[polyIdx][planeId].vert[nextv].y;
        }

        Color c = GetColorByDistance(screenSpacePolys[polyIdx]->distFromCamera);
        SDL_SetRenderDrawColor(renderer, c.R, c.G, c.B, 255);
 
        for (int y = 0; y < screenH; y += RASTER_RESOLUTION) {
            for (int x = 0; x < screenW; x += 1) {
                if (pixelBuff[y][x] == 1) continue;
 
                if (PointInPoly(RASTER_NUM_VERTS, vx, vy, x, y) == 1) {
                    // for (int learp = 0; learp < RASTER_RESOLUTION; learp++) PutPixel(x, y + learp, 100, 255, 0);
 
                    PutPixel(x, y, c.R, c.G, c.B);
                    pixelBuff[y][x] = 1;
                }
            }
        }
    }
}


float ClosestVertexInPoly(Polygon poly, Vec2 pos) {
    float dist = 9999999;
    for (int i = 0; i < poly.vertCnt; i++) {
        float d = Len(pos, poly.vert[i]);
        if (d < dist) dist = d;
    }
    
    return dist;
}

void SortPolysByDepth() {
    for(int i=0; i < MAX_POLYS; i++) {
        for(int j=0; j < MAX_POLYS - i - 1; j++) {
            Polygon poly1 = polys[j];
            Polygon poly2 = polys[j+1];
            
            float distP1 = ClosestVertexInPoly(poly1, cam.camPos);
            float distP2 = ClosestVertexInPoly(poly2, cam.camPos);
            
            polys[j].curDist = distP1;
            polys[j+1].curDist = distP2;
            
            if(distP1 < distP2) {
                Polygon temp = polys[j+1];
                polys[j+1] = polys[j];
                polys[j] = temp;
            }
        }
    }
}

void Render() {
    SortPolysByDepth();
    
    if (SHOULD_RASTERIZE == 1) {
        ClearRasterBuffer();
        screenSpaceVisiblePlanes = 0;
    }
    
    for (int polyIdx = 0; polyIdx < MAX_POLYS; polyIdx++) {    
        for (int i = 0; i < polys[polyIdx].vertCnt - 1; i++) {
            Vec2 p1 = polys[polyIdx].vert[i];
            Vec2 p2 = polys[polyIdx].vert[i + 1];
            float height = -polys[polyIdx].height / RES_DIV;
            
            if (IsFrontFace(cam.camPos , p1, p2) > 0) continue;;
            
            float distX1 = p1.x - cam.camPos.x;
            float distY1 = p1.y - cam.camPos.y;
            float z1 = distX1 * cos(cam.camAngle) + distY1 * sin(cam.camAngle);
            
            float distX2 = p2.x - cam.camPos.x;
            float distY2 = p2.y - cam.camPos.y;
            float z2 = distX2 * cos(cam.camAngle) + distY2 * sin(cam.camAngle);
            
            distX1 = distX1 * sin(cam.camAngle) - distY1 * cos(cam.camAngle);
            distX2 = distX2 * sin(cam.camAngle) - distY2 * cos(cam.camAngle);
            
            const float NEAR_CLIP = 0.1f;
            
            // Reject if the whole segment is behind the near plane
            if (z1 <= NEAR_CLIP && z2 <= NEAR_CLIP) continue;
            
            // If one endpoint is behind, clip it to z = NEAR_CLIP
            if (z1 < NEAR_CLIP) {
                float t = (NEAR_CLIP - z1) / (z2 - z1);
                distX1 = distX1 + t * (distX2 - distX1);
                z1 = NEAR_CLIP;
            }
            if (z2 < NEAR_CLIP) {
                float t = (NEAR_CLIP - z2) / (z1 - z2);
                distX2 = distX2 + t * (distX1 - distX2);
                z2 = NEAR_CLIP;
            }
            
            // Safety clamp
            z1 = (z1 < NEAR_CLIP) ? NEAR_CLIP : z1;
            z2 = (z2 < NEAR_CLIP) ? NEAR_CLIP : z2;
            
            float widthRatio = screenW / 2.0f;
            float heightRatio = (static_cast<float>(screenW) * static_cast<float>(screenH)) / 60.0f;
            float centerScreenH = screenH / 2.0f;
            float centerScreenW = screenW / 2.0f;
            
            float x1 = -distX1 * widthRatio / z1;
            float x2 = -distX2 * widthRatio / z2;
            float y1a = (height - heightRatio) / z1;
            float y1b = heightRatio / z1;
            float y2a = (height - heightRatio) / z2;
            float y2b = heightRatio / z2;
            
            // Draws wireframe
            // DrawLine(centerScreenW + x1, centerScreenH + y1a, centerScreenW + x2, centerScreenH + y2a);
            // DrawLine(centerScreenW + x1, centerScreenH + y1b, centerScreenW + x2, centerScreenH + y2b);
            // DrawLine(centerScreenW + x1, centerScreenH + y1a, centerScreenW + x1, centerScreenH + y1b);
            // DrawLine(centerScreenW + x2, centerScreenH + y2a, centerScreenW + x2, centerScreenH + y2b);
            
            //wave player if walking
            float wave = WWAVE_MAG * sinf(cam.stepWave);
            y1a += wave, y1b += wave, y2a += wave, y2b += wave;
            
            // Fill the rasterization buffer
            if (SHOULD_RASTERIZE == 1) {
                int planeIdx = screenSpaceVisiblePlanes;
                
                screenSpacePolys[planeIdx][i].vert[0].x = centerScreenW + x2;
                screenSpacePolys[planeIdx][i].vert[0].y = centerScreenH + y2a;
                screenSpacePolys[planeIdx][i].vert[1].x = centerScreenW + x1;
                screenSpacePolys[planeIdx][i].vert[1].y = centerScreenH + y1a;
                screenSpacePolys[planeIdx][i].vert[2].x = centerScreenW + x1;
                screenSpacePolys[planeIdx][i].vert[2].y = centerScreenH + y1b;
                screenSpacePolys[planeIdx][i].vert[3].x = centerScreenW + x2;
                screenSpacePolys[planeIdx][i].vert[3].y = centerScreenH + y2b;
                
                screenSpacePolys[planeIdx]->planeIdInPoly = i;
                screenSpacePolys[planeIdx]->distFromCamera = (z1 + z2) / 2;
                screenSpaceVisiblePlanes++;
            }
        }
    }
    
    if (SHOULD_RASTERIZE == 1) Rasterize();  
}

void Init() {
    cam.camAngle = 0.42;
    cam.camPos.x = 451.96;
    cam.camPos.y = 209.24;
 
    polys[0].vert[0].x = 141.00;
    polys[0].vert[0].y = 84.00;
    polys[0].vert[1].x = 496.00;
    polys[0].vert[1].y = 81.00;
    polys[0].vert[2].x = 553.00;
    polys[0].vert[2].y = 136.00;
    polys[0].vert[3].x = 135.00;
    polys[0].vert[3].y = 132.00;
    polys[0].vert[4].x = 141.00;
    polys[0].vert[4].y = 84.00;
    polys[0].height = 50000;
    polys[0].vertCnt = 5;
    polys[1].vert[0].x = 133.00;
    polys[1].vert[0].y = 441.00;
    polys[1].vert[1].x = 576.00;
    polys[1].vert[1].y = 438.00;
    polys[1].vert[2].x = 519.00;
    polys[1].vert[2].y = 493.00;
    polys[1].vert[3].x = 123.00;
    polys[1].vert[3].y = 497.00;
    polys[1].vert[4].x = 133.00;
    polys[1].vert[4].y = 441.00;
    polys[1].height = 50000;
    polys[1].vertCnt = 5;
    polys[2].vert[0].x = 691.00;
    polys[2].vert[0].y = 165.00;
    polys[2].vert[1].x = 736.00;
    polys[2].vert[1].y = 183.00;
    polys[2].vert[2].x = 737.00;
    polys[2].vert[2].y = 229.00;
    polys[2].vert[3].x = 697.00;
    polys[2].vert[3].y = 247.00;
    polys[2].vert[4].x = 656.00;
    polys[2].vert[4].y = 222.00;
    polys[2].vert[5].x = 653.00;
    polys[2].vert[5].y = 183.00;
    polys[2].vert[6].x = 691.00;
    polys[2].vert[6].y = 165.00;
    polys[2].height = 10000;
    polys[2].vertCnt = 7;
    polys[3].vert[0].x = 698.00;
    polys[3].vert[0].y = 330.00;
    polys[3].vert[1].x = 741.00;
    polys[3].vert[1].y = 350.00;
    polys[3].vert[2].x = 740.00;
    polys[3].vert[2].y = 392.00;
    polys[3].vert[3].x = 699.00;
    polys[3].vert[3].y = 414.00;
    polys[3].vert[4].x = 654.00;
    polys[3].vert[4].y = 384.00;
    polys[3].vert[5].x = 652.00;
    polys[3].vert[5].y = 348.00;
    polys[3].vert[6].x = 698.00;
    polys[3].vert[6].y = 330.00;
    polys[3].height = 10000;
    polys[3].vertCnt = 7;
    polys[4].vert[0].x = 419.00;
    polys[4].vert[0].y = 311.00;
    polys[4].vert[1].x = 461.00;
    polys[4].vert[1].y = 311.00;
    polys[4].vert[2].x = 404.00;
    polys[4].vert[2].y = 397.00;
    polys[4].vert[3].x = 346.00;
    polys[4].vert[3].y = 395.00;
    polys[4].vert[4].x = 348.00;
    polys[4].vert[4].y = 337.00;
    polys[4].vert[5].x = 419.00;
    polys[4].vert[5].y = 311.00;
    polys[4].height = 50000;
    polys[4].vertCnt = 6;
    polys[5].vert[0].x = 897.00;
    polys[5].vert[0].y = 98.00;
    polys[5].vert[1].x = 1079.00;
    polys[5].vert[1].y = 294.00;
    polys[5].vert[2].x = 1028.00;
    polys[5].vert[2].y = 297.00;
    polys[5].vert[3].x = 851.00;
    polys[5].vert[3].y = 96.00;
    polys[5].vert[4].x = 897.00;
    polys[5].vert[4].y = 98.00;
    polys[5].height = 10000;
    polys[5].vertCnt = 5;
    polys[6].vert[0].x = 1025.00;
    polys[6].vert[0].y = 294.00;
    polys[6].vert[1].x = 1080.00;
    polys[6].vert[1].y = 292.00;
    polys[6].vert[2].x = 1149.00;
    polys[6].vert[2].y = 485.00;
    polys[6].vert[3].x = 1072.00;
    polys[6].vert[3].y = 485.00;
    polys[6].vert[4].x = 1025.00;
    polys[6].vert[4].y = 294.00;
    polys[6].height = 1000;
    polys[6].vertCnt = 5;
    polys[7].vert[0].x = 1070.00;
    polys[7].vert[0].y = 483.00;
    polys[7].vert[1].x = 1148.00;
    polys[7].vert[1].y = 484.00;
    polys[7].vert[2].x = 913.00;
    polys[7].vert[2].y = 717.00;
    polys[7].vert[3].x = 847.00;
    polys[7].vert[3].y = 718.00;
    polys[7].vert[4].x = 1070.00;
    polys[7].vert[4].y = 483.00;
    polys[7].height = 1000;
    polys[7].vertCnt = 5;
    polys[8].vert[0].x = 690.00;
    polys[8].vert[0].y = 658.00;
    polys[8].vert[1].x = 807.00;
    polys[8].vert[1].y = 789.00;
    polys[8].vert[2].x = 564.00;
    polys[8].vert[2].y = 789.00;
    polys[8].vert[3].x = 690.00;
    polys[8].vert[3].y = 658.00;
    polys[8].height = 10000;
    polys[8].vertCnt = 4;
    polys[9].vert[0].x = 1306.00;
    polys[9].vert[0].y = 598.00;
    polys[9].vert[1].x = 1366.00;
    polys[9].vert[1].y = 624.00;
    polys[9].vert[2].x = 1369.00;
    polys[9].vert[2].y = 678.00;
    polys[9].vert[3].x = 1306.00;
    polys[9].vert[3].y = 713.00;
    polys[9].vert[4].x = 1245.00;
    polys[9].vert[4].y = 673.00;
    polys[9].vert[5].x = 1242.00;
    polys[9].vert[5].y = 623.00;
    polys[9].vert[6].x = 1306.00;
    polys[9].vert[6].y = 598.00;
    polys[9].height = 50000;
    polys[9].vertCnt = 7;
}

void UpdateScreen() {
    SDL_RenderPresent(renderer);
}

//-- Physics --
int LineCircleCollision(LineSeg line, Vec2 circleCenter, float circleRadius)
{
    Vec2 closestPointToLine = ClosestPointOnLine(line, circleCenter);
    int isClosestPointOnLine = IsPointOnLine(line, closestPointToLine);
 
    if (isClosestPointOnLine == 0) return 0;
 
    float circleToPointOnLineDist = Len(closestPointToLine, circleCenter);
   
    if (circleToPointOnLineDist < circleRadius) return 1;
 
    return 0;
}
 
Vec2 ResolveCollision(Vec2 lastPosition, Vec2 currentPosition, LineSeg lineOfCollision, float deltaTime) {
    Vec2 dir = VecMinus(currentPosition, lastPosition);
    Vec2 collisionPoint = ClosestPointOnLine(lineOfCollision, currentPosition);
    Vec2 collisionDir = VecMinus(collisionPoint, currentPosition);
   
    Vec2 n = Normalize(collisionDir);
    float dot = Dot(dir, n);
    n = VecMulF(n, dot);
    dir.x -= n.x;
    dir.y -= n.y;
 
    Vec2 resolvedPos = VecPlus(lastPosition, dir);
 
    return resolvedPos;
}

void CollisionDetection(float deltaTime) {
    float radius = 10.0f;
 
    for (int polyIdx = 0; polyIdx < MAX_POLYS; polyIdx++) {
        for (int i = 0; i < polys[polyIdx].vertCnt - 1; i++) {
            Vec2 p1 = polys[polyIdx].vert[i];
            Vec2 p2 = polys[polyIdx].vert[i + 1];
 
            LineSeg line;
            line.p1 = p1;
            line.p2 = p2;
 
            int collision =
                LineCircleCollision(line, cam.camPos, radius);            
            if (collision != 0) {
                cam.camPos =
                    ResolveCollision(cam.oldCamPos,
                    cam.camPos, line, deltaTime);
            }
        }
    }
}

// -- Math --
float DotPoints(float x1, float y1, float x2, float y2) {
    return x1 * x2 + y1 * y2;
}
 
float Dot(Vec2 pointA, Vec2 pointB) {
    return DotPoints(pointA.x, pointA.y, pointB.x, pointB.y);
}
 
Vec2 Normalize(Vec2 vec) {
    float len = sqrt((vec.x * vec.x) + (vec.y * vec.y));
    Vec2 normalized;
    normalized.x = vec.x / len;
    normalized.y = vec.y / len;
 
    return normalized;
}
 
Vec2 VecMinus(Vec2 v1, Vec2 v2) {
    Vec2 v3;
    v3.x = v1.x - v2.x;
    v3.y = v1.y - v2.y;
 
    return v3;
}
 
Vec2 VecPlus(Vec2 v1, Vec2 v2) {
    Vec2 v3;
    v3.x = v1.x + v2.x;
    v3.y = v1.y + v2.y;
 
    return v3;
}
 
Vec2 VecMulF(Vec2 v1, float val) {
    Vec2 v2;
    v2.x = v1.x * val;
    v2.y = v1.y * val;
 
    return v2;
}
 
float Len(Vec2 pointA, Vec2 pointB) {
    float distX = pointB.x - pointA.x;
    float distY = pointB.y - pointA.y;
 
    return sqrt((distX * distX) + (distY * distY));
}