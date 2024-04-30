#include <raylib.h>
#include "TriangleNet.h"
#include <raymath.h>
#include <vector>
using namespace std;

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "TriangleNets");
    SetConfigFlags(FLAG_VSYNC_HINT);

    TriangleNet net;
    net.position = { 400, 400 };
    vector<Vector2> verts = {
        { 0, 0 },
        { 1, 0 },
        { 1, 1 },
        { 0, 0 },
        { 0, 1 },
        { 1, 1 }

    };
    net.AddTriangles(verts);

    vector<Vector2> selectedVerts;
    vector<Vector2> polygon = net.GetPolygon();
    Vector2 mouseInv = {};
    Vector2 nearestVertex = {};
    float snapDistance = 0.3f;

    float timer = 0;
    int untilCounter = 0;

    while(!WindowShouldClose()) {
        // Get nearest and draw UI stuff.
        mouseInv = net.InvTransform(GetMousePosition());
        nearestVertex = net.GetNearestVertex(mouseInv, snapDistance);
        if (timer > 0.3) {
            untilCounter = (untilCounter + 1) % (polygon.size()+1);
            timer = 0;
        }
        timer += GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        //GuiGrid({ 0, 0, 800, 800 }, "", net.scale, 4, 0);
        net.Draw(BLUE, RED);
        DrawCircleLinesV(net.Transform(nearestVertex), 5.0f, GREEN);
        DrawCircleLinesV(GetMousePosition(), net.scale*snapDistance, DARKGREEN);
        net.DrawLabels(20.0f, DARKGRAY);  
        net.DrawPolygon(selectedVerts, GREEN);
        net.DrawPolygon(polygon, MAGENTA, untilCounter);

        // Add this vertex to the selection list.
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedVerts.push_back(nearestVertex);
            if (selectedVerts.size() == 3) {
                net.AddTriangles(selectedVerts);
                polygon = net.GetPolygon();
                selectedVerts.clear();
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            float minDist = 9999;
            int nearest = 0;
            for (int i = 0; i < selectedVerts.size(); i++) {
                float dist = Vector2Distance(mouseInv, selectedVerts[i]);
                if (dist < minDist) {
                    nearest = i;
                    minDist = dist;
                }
            }
            selectedVerts.erase(selectedVerts.begin()+nearest);
        }

        DrawText(TextFormat("%3.2f %3.2f %d", nearestVertex.x, nearestVertex.y, untilCounter), 10, 10, 20.0f, DARKGRAY);
        for (int i =0 ; i < polygon.size(); i++){
            Vector2 vert = polygon[i]; 
            DrawText(TextFormat("%3.2f, %3.2f", vert.x, vert.y), 600, 10+20*i, 20.0f, WHITE);
        }
        EndDrawing();
    }
}