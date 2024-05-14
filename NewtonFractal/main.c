#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "ComPlex");
    SetTargetFPS(144);

    Texture2D blankTex; {
        Image im = GenImageColor(1, 1, WHITE);
        blankTex = LoadTextureFromImage(im);
    }

    Shader coolShader = LoadShader(0, "pixel.fs");

    int maxIterLoc = GetShaderLocation(coolShader, "maxIterations");
    int timeLoc = GetShaderLocation(coolShader, "time");
    float maxIterPercent = 0.3f;
    int maxIter = 100;
    int curIter = 100;

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        curIter = (int)(maxIter * maxIterPercent);
        float time = GetTime();
        SetShaderValue(coolShader, maxIterLoc, &curIter, SHADER_UNIFORM_INT);
        SetShaderValue(coolShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

        BeginShaderMode(coolShader);
        DrawTexturePro(blankTex, (Rectangle){ 0, 0, 1, 1 }, 
            (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, 
            (Vector2){ 0, 0 }, 0, WHITE);
        EndShaderMode();

        GuiSlider((Rectangle){ 40, 10, 200, 10 }, "Iter", TextFormat("%d", curIter), &maxIterPercent, 0, 1);

        EndDrawing();
    }
}