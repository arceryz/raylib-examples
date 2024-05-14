#include <raylib.h>
#include <vector>
#include <raymath.h>
using namespace std;

class Polygon {
    vector<Vector2> points;
    Vector2 center;
public:
    Vector2 origin;
    float size;

    Polygon() {};
    void LoadShape(int shapeNr) {
        shapeNr = shapeNr % 1;
        switch(shapeNr) {
            case 0: 
            {
                vector<Vector2> _points = {
                    { 1, 0 },
                    { 0, 1 },
                    { -1, 0 },
                    { 0, -1 }
                };
                points = _points;
            }
        }
        ComputeCenter();
    }
    void Draw(bool filled)
    { 
        for (int i = 0; i < points.size(); i++) {
            Vector2 u = points[i];
            Vector2 v = points[(i+1)%points.size()];
            DrawLineV(Transform(u), Transform(v), BLUE);
        }
        vector<Vector2> fan = { Transform(center) };
        for (Vector2 u: points) {
            Vector2 tu = Transform(u);
            DrawCircleV(tu, 5, GREEN);
            fan.push_back(tu);
        }
        fan.push_back(fan[1]);

        if (filled) {
            DrawTriangleFan(fan.data(), fan.size(), Fade(BLUE, 0.2));
        }
    }
    float GetWindingDegrees(Vector2 point)
    {
        float winding = 0;
        for (int i = 0; i < points.size(); i++) {
            Vector2 u = points[i];
            Vector2 v = points[(i+1)%points.size()];
            Vector2 pu = Vector2Normalize(Vector2Subtract(u, point));
            Vector2 pv = Vector2Normalize(Vector2Subtract(v, point));
            
            // cos(a) = <pu, pv> / |pu||pv|
            float angle = acosf(Vector2DotProduct(pu, pv));
            winding += angle;
        }
        return RAD2DEG*winding;
    }

    void DrawAngleLines(Vector2 point)
    {
        Vector2 p = Transform(point);
        vector<float> angles;
        angles.resize(points.size());
        float winding = 0;

        for (int i = 0; i < points.size(); i++) {
            Vector2 u = points[i];
            Vector2 v = points[(i+1)%points.size()];
            Vector2 pu = Vector2Normalize(Vector2Subtract(u, point));
            Vector2 pv = Vector2Normalize(Vector2Subtract(v, point));
            
            // cos(a) = <pu, pv> / |pu||pv|
            float angle = acosf(Vector2DotProduct(pu, pv));
            angles[i] = angle;
            winding += angle;
            DrawLineV(p, Transform(u), Fade(RED, 0.8));
        }
        for (int i = 0; i < points.size(); i++) {
            Vector2 u = Transform(points[i]);
            Vector2 v = Transform(points[(i+1)%points.size()]);
            Vector2 avg = Vector2Scale(Vector2Add(p, Vector2Add(u, v)), 0.333f);

            float ang = RAD2DEG*angles[i];
            DrawText(TextFormat("%.0f", ang), avg.x, avg.y-20, 20, MAGENTA);
        }
        DrawText(TextFormat("%.0f", RAD2DEG*winding), p.x-15, p.y-20, 20, MAGENTA);
    }
    void ComputeCenter() 
    {
        center = {};
        for (Vector2 u: points) { center = Vector2Add(center, u); }
        Vector2Scale(center, 1.0/points.size());
    }
    bool IsPointInside(Vector2 point)
    {
        return GetWindingDegrees(point) >= 359.9;
    }

    Vector2 Transform(Vector2 point)
    {
        return Vector2Add(origin, Vector2Multiply(point, { size, -size }));
    }
    Vector2 InvTransform(Vector2 point)
    {
        return Vector2Multiply(Vector2Subtract(point, origin), { 1.0f/size, -1.0f/size } );
    }
};

int main() {
    InitWindow(800, 800, "PointIn");
    Polygon polygon;
    polygon.origin = { 400, 400 };
    polygon.size = 250;
    polygon.LoadShape(0);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        Vector2 mouseLocal = polygon.InvTransform(GetMousePosition());
        bool inside = polygon.IsPointInside(mouseLocal);
        polygon.Draw(inside);
        polygon.DrawAngleLines(mouseLocal);

        EndDrawing();
    }
}