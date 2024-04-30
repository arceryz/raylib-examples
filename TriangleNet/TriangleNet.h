#ifndef TRIANGLE_NET_H
#define TRIANGLE_NET_H

#include <raylib.h>
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;

// A triangle net class is used visualise the triangle merging problem
// and to turn triangles into polygons (list of vertices).
class TriangleNet
{
private:

public:
    Vector2 position = {};
    float scale = 100.0f;

    vector<Vector2> vertices;
    vector<int> indices;
    unordered_map<string, int> vertexToIndex;
    unordered_map<int, unordered_map<int, int>> indexToNeighbors;
    
    TriangleNet();
    void Clear();
    // Load the data from a plain triangle list.
    // Triangles are then merged by distance and indexed.
    void AddTriangles(vector<Vector2> triangles);
    bool IsEdgeInternal(int u, int v);
    bool IsVertexInternal(int u);
    vector<int> GetPolygonRecursive(int currentVertex, unordered_map<int, bool> &seen);
    vector<Vector2> GetPolygon();
    void Draw(Color internal, Color external);
    void DrawPolygon(vector<Vector2> verts, Color color, int until=-1);
    void DrawLabels(float size, Color color);
    string GetVertexKey(Vector2 vert);
    Vector2 Transform(Vector2 vert);
    Vector2 InvTransform(Vector2 vert);
    Vector2 GetNearestVertex(Vector2 pos, float dist);
};

#endif