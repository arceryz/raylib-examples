#include "TriangleNet.h"
#include <raymath.h>

TriangleNet::TriangleNet()
{

}
void TriangleNet::Clear()
{
    vertices.clear();
    vertexToIndex.clear();
    indices.clear();
    indexToNeighbors.clear();
}
void TriangleNet::AddTriangles(vector<Vector2> triangles)
{
    // Iterate every triangle.
    for (int i = 0; i < triangles.size(); i += 3) {
        for (int j = 0; j < 3; j++) { 
            Vector2 vertex = triangles[i+j];
            string key = GetVertexKey(vertex);
            // Add a new vertex if this key is not present.
            if (vertexToIndex.find(key) == vertexToIndex.end()) {
                vertices.push_back(vertex);
                vertexToIndex[key] = vertices.size()-1;
            } 
            indices.push_back(vertexToIndex[key]);
        }

        // Count the number of times each edge is added.
        for (int j = indices.size()-3; j < indices.size(); j++) {
            for (int k = indices.size()-3; k < indices.size(); k++) {
                if (j == k) continue;
                indexToNeighbors[indices[j]][indices[k]] += 1;
            }
        }
    }
}
bool TriangleNet::IsEdgeInternal(int u, int v)
{
    return indexToNeighbors[u][v] != 1;
}
bool TriangleNet::IsVertexInternal(int u)
{
    for (auto kv: indexToNeighbors[u]) {
        if (kv.second < 2)
            return false;
    }
    return true;
}
string TriangleNet::GetVertexKey(Vector2 vert)
{
    return string(TextFormat("%3.3f,%3.3f", vert.x, vert.y));
}
void TriangleNet::Draw(Color external, Color internal)
{
    for (int i = 0; i < indices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int i1 = indices[i+j];
            int i2 = indices[i+(j+1)%3];
            Vector2 v1 = Transform(vertices[i1]);
            Vector2 v2 = Transform(vertices[i2]);
            DrawLineV(v1, v2, IsEdgeInternal(i1, i2) ? internal: external);
        }
    }
    for (int i = 0; i < vertices.size(); i++) {
        DrawCircleV(Transform(vertices[i]), 3, IsVertexInternal(i) ? internal: external);
    }
}
void TriangleNet::DrawPolygon(vector<Vector2> verts, Color color, int until)
{
    if (until < 0) {
        until = verts.size();
    }
    // if until==0 then we draw a single vert.
    for (int i = 0; i <= until && i < verts.size(); i++) {
        DrawCircleV(Transform(verts[i]), 4.0f, color);
    }
    // Draw edges stopped at until.
    for (int i = 0; i < until; i++) {
        Vector2 v1 = Transform(verts[i]);
        Vector2 v2 = Transform(verts[(i+1)%verts.size()]);
        DrawLineV(v1, v2, color);
    }
}
void TriangleNet::DrawLabels(float size, Color color)
{
    for (int i = 0; i < vertices.size(); i++) {
        Vector2 pos = Transform(vertices[i]);
        const char *text = TextFormat("%d\n", i);
        DrawText(text, pos.x, pos.y, size, color);
    }
    // Draw all edge counters.
    for (auto kv1: indexToNeighbors) {
        for (auto kv2: kv1.second) {
            if (kv2.second == 0) continue;
            Vector2 pos1 = vertices[kv1.first];
            Vector2 pos2 = vertices[kv2.first];
            Vector2 dir = Vector2Scale(Vector2Subtract(pos2, pos1), 0.3f);
            Vector2 textPos = Transform(Vector2Add(pos1, dir));
            DrawText(TextFormat("%d", kv2.second), textPos.x, textPos.y, size*0.5, color);
        }
    }
}
Vector2 TriangleNet::Transform(Vector2 vert)
{
    return Vector2Add(Vector2Scale({ vert.x, -vert.y }, scale), position);
}
Vector2 TriangleNet::InvTransform(Vector2 pos)
{
    Vector2 vec = Vector2Scale(Vector2Subtract(pos, position), 1.0/scale);
    return { vec.x, -vec.y };
}
Vector2 TriangleNet::GetNearestVertex(Vector2 pos, float dist)
{
    float minDist = 99999;
    Vector2 minVert;
    for (Vector2 vert: vertices) {
        float dist = Vector2Distance(pos, vert);
        if (dist < minDist) {
            minDist = dist;
            minVert = vert;
        }
    }
    return minDist < dist ? minVert: pos;
}
vector<Vector2> TriangleNet::GetPolygon()
{
    // Find starting vertex.
    int currentVertex = 0;
    for (int i=0; i < vertices.size(); i++) {
        if (!IsVertexInternal(i)) {
            currentVertex = i;
            break;
        }
    }
    
    // Now traverse all verts starting from this one,
    // ignoring those we have already seen.
    unordered_map<int, bool> seen;
    vector<Vector2> list;
    
    for (int i = 0; i < vertices.size(); i++){ 
        seen[currentVertex] = true;
        list.push_back(vertices[currentVertex]);

        int nextVertex = -1;
        for (auto kv: indexToNeighbors[currentVertex]) {
            if (kv.second == 1 && !seen[kv.first]) {
                nextVertex = kv.first;
                break;
            }
        }
        if (nextVertex == -1) {
            break;
        }
        currentVertex = nextVertex;
    }

    return list;
}