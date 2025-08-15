#include "typedefs.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>

#undef main

#define screenW 1152
#define screenH 758

SDL_Renderer* renderer;

Camera cam;
Polygon polys[MAX_POLYS];

void Init();
void Render();
void UpdateScreen();
int ShouldQuit(SDL_Event event);

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* mainWin = SDL_CreateWindow(
        "knock-off doom",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenW, screenH,
        SDL_WINDOW_SHOWN
    );
 
    renderer = SDL_CreateRenderer(mainWin, 0, SDL_RENDERER_SOFTWARE);

    Init();

    int loop = 1;
    SDL_Event event;
    while (loop) {
        SDL_PollEvent(&event);

        // Clear window color to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        Render();
        UpdateScreen();

        if (ShouldQuit(event)) break;
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

void Render() {
    for (int polyIdx = 0; polyIdx < MAX_POLYS; polyIdx++) {    
        for (int i = 0; i < polys[polyIdx].vertCnt - 1; i++) {
            Vec2 p1 = polys[polyIdx].vert[i];
            Vec2 p2 = polys[polyIdx].vert[i + 1];
            float height = -polys[polyIdx].height;

            if (IsFrontFace(cam.camPos , p1, p2) > 0) continue;;

            float distX1 = p1.x - cam.camPos.x;
            float distY1 = p1.y - cam.camPos.y;
            float z1 = distX1 * cos(cam.camAngle) + distY1 * sin(cam.camAngle);

            float distX2 = p2.x - cam.camPos.x;
            float distY2 = p2.y - cam.camPos.y;
            float z2 = distX2 * cos(cam.camAngle) + distY2 * sin(cam.camAngle);

            distX1 = distX1 * sin(cam.camAngle) - distY1 * cos(cam.camAngle);
            distX2 = distX2 * sin(cam.camAngle) - distY2 * cos(cam.camAngle);

            if (z1 > 0 || z2 > 0) {
                Vec2 i1 = Intersection(distX1, z1, distX2, z2, -0.0001, 0.0001, -20, 5);
                Vec2 i2 = Intersection(distX1, z1, distX2, z2, 0.0001, 0.0001, 20, 5);

                if (z1 <= 0) {
                    if (i1.y > 0) { distX1 = i1.x; z1 = i1.y; }
                    else { distX1 = i2.x; z1 = i2.y; }
                }

                if (z2 <= 0) {
                    if (i1.y > 0) { distX2 = i1.x; z2 = i1.y; }
                    else { distX2 = i2.x; z2 = i2.y; }
                }
            } else {
                continue;
            }

            float widthRatio = screenW / 2.0f;
            float heightRatio = (screenW * screenH) / 60.0f;
            float centerScreenH = screenH / 2.0f;
            float centerScreenW = screenW / 2.0f;

            float x1 = -distX1 * widthRatio / z1;
            float x2 = -distX2 * widthRatio / z2;
            float y1a = (height - heightRatio) / z1;
            float y1b = heightRatio / z1;
            float y2a = (height - heightRatio) / z2;
            float y2b = heightRatio / z2;

            DrawLine(centerScreenW + x1, centerScreenH + y1a, centerScreenW + x2, centerScreenH + y2a);
            DrawLine(centerScreenW + x1, centerScreenH + y1b, centerScreenW + x2, centerScreenH + y2b);
            DrawLine(centerScreenW + x1, centerScreenH + y1a, centerScreenW + x1, centerScreenH + y1b);
            DrawLine(centerScreenW + x2, centerScreenH + y2a, centerScreenW + x2, centerScreenH + y2b);
        }
    }
}

void UpdateScreen() {
    SDL_RenderPresent(renderer);
}