#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

Matrix GetCameraProjectionMatrix(Camera3D camera, float near, float far)
{
    Matrix proj;
    float aspect = ((float)GetScreenWidth()) / GetScreenHeight();
    if (camera.projection == CAMERA_PERSPECTIVE) {
        proj = MatrixPerspective(camera.fovy*DEG2RAD, aspect, near, far);
    } else {
        // Orthographic projection has Y range [-fov/2, fov/2] and xrange aspect*Yrange.
        double top = camera.fovy/2.0;
        double right = top*aspect;
        proj = MatrixOrtho(-right, right, -top, top, near, far);
    }
    return proj;
}

struct Line3D {
    Vector3 start;
    Vector3 end;
};

Line3D GetCameraWorldRay(Vector3 ndc, Matrix view, Matrix proj)
{
    // Return the view ray in world space.
    Matrix invProjView = MatrixInvert(MatrixMultiply(view, proj));
    Line3D line;

    Quaternion worldQuat1 = QuaternionTransform({ ndc.x, ndc.y, -1, 1 }, invProjView);
    Quaternion worldQuat2 = QuaternionTransform({ ndc.x, ndc.y, 1, 1 }, invProjView);
    worldQuat1 = QuaternionScale(worldQuat1, 1.0f/worldQuat1.w);
    worldQuat2 = QuaternionScale(worldQuat2, 1.0f/worldQuat2.w);

    line.start = { worldQuat1.x, worldQuat1.y, worldQuat1.z };
    line.end = { worldQuat2.x, worldQuat2.y, worldQuat2.z };

    line.start = Vector3Unproject({ ndc.x, ndc.y, -1 }, proj, view);
    line.end = Vector3Unproject({ ndc.x, ndc.y, 1 }, proj, view);
    return line;
}

void DrawCameraFrustrum(Camera3D mainCamera, Camera3D camera, float near, float far)
{
    // There are 8 points we need to unproject to draw the frustum.
    // They are the 8 corners of the frustum box.
    // The near points.
    Vector3 ndcPoints[8] = {
        { -1,  1, -1 },
        {  1,  1, -1 },
        { -1, -1, -1 },
        {  1, -1, -1 },
        { -1,  1,  1 },
        {  1,  1,  1 },
        { -1, -1,  1 },
        {  1, -1,  1 }
    };

    // We get the internal projection matrix by switching to 3D mode real quick.
    Matrix view = GetCameraMatrix(camera);
    Matrix proj = GetCameraProjectionMatrix(camera, near, far);
    Matrix invView = MatrixInvert(view);
    Matrix invProj = MatrixInvert(proj);

    Vector3 worldPoints[8];
    for (int i = 0; i < 8; i++) {
        Vector3 ndc = ndcPoints[i];
        //ndc.z *= 0.5 + 0.4*sin(GetTime()*10);
        Vector3 clip = ndc;

        // For projection we scale by W coordinate.
        float w = camera.projection == CAMERA_PERSPECTIVE ? (ndc.z < 0 ? near: far) : 1;
        clip = Vector3Scale(clip, w);

        // Since clip = PROJ * view, we find view by multiplying clip by PROJ^-1.
        // We must turn our Vector3 to a Vector4(=Quaternion in Raylib terms).
        Quaternion clipQuat = { clip.x, clip.y, clip.z, w };
        Quaternion viewQuat = QuaternionTransform(clipQuat, invProj);
        Vector3 view = { viewQuat.x, viewQuat.y, viewQuat.z };

        // Since we now have the view space position, we must multiply by
        // the inverse view matrix to get the resulting world position.
        Vector3 world = Vector3Transform(view, invView);

        worldPoints[i] = world;
    }

    // Draw these points and lines in world space.
    DrawSphere(camera.position, 0.02f, GREEN);
    for (int i = 0; i < 8; i++) {
        Vector3 world1 = worldPoints[i];
        Vector3 ndc1 = ndcPoints[i];

        for (int j = 0; j < 8; j++) {
            Vector3 ndc2 = ndcPoints[j];
            int diff = (int)(ndc1.x!=ndc2.x) + (int)(ndc1.y!=ndc2.y) + (int)(ndc1.z!=ndc2.z);
            if (diff == 1) {
                Vector3 world2 = worldPoints[j];
                DrawLine3D(world1, world2, RED);
            }
        }
    }
    Color color = Fade(RED, 0.4f);
    DrawTriangle3D(worldPoints[0], worldPoints[2], worldPoints[1], color);
    DrawTriangle3D(worldPoints[1], worldPoints[2], worldPoints[0], color);
    DrawTriangle3D(worldPoints[2], worldPoints[3], worldPoints[1], color);
    DrawTriangle3D(worldPoints[1], worldPoints[3], worldPoints[2], color);

    DrawTriangle3D(worldPoints[4], worldPoints[6], worldPoints[5], color);
    DrawTriangle3D(worldPoints[5], worldPoints[6], worldPoints[4], color);
    DrawTriangle3D(worldPoints[6], worldPoints[7], worldPoints[5], color);
    DrawTriangle3D(worldPoints[5], worldPoints[7], worldPoints[6], color);
}

int main()
{
    InitWindow(800, 800, "Unproject");

    Camera3D camera;
    camera.position = { 1, 1, 1 };
    camera.target = { 0, 0.25, 0 };
    camera.fovy = 45.0f;
    camera.up = { 0, 1, 0 };
    camera.projection = CAMERA_PERSPECTIVE;

    Camera3D mainCamera;
    mainCamera.position = { 4, 4, 4 };
    mainCamera.target = { };
    mainCamera.fovy = 45.0f;
    mainCamera.up = { 0, 1, 0 };
    mainCamera.projection = CAMERA_PERSPECTIVE;

    float nearPlane = 0.1f;
    float farPlane = 2.0f;
    float fovy = 45.0f;
    float extents = 1.0f;
    bool orthoGraphic = false;

    RenderTexture2D renderTexture = LoadRenderTexture(800, 800);
    Texture2D smileyTexture = LoadTexture("smiley.png");

    while(!WindowShouldClose()){
        UpdateCamera(&camera, CAMERA_ORBITAL);
        camera.fovy = orthoGraphic ? extents: fovy;
        float time = GetTime();
        Vector2 mousePos = GetMousePosition();

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the perspective of the debug camera.
        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        BeginMode3D(camera);

        // Set our custom projection to have custom near and far plane too.
        Matrix proj = GetCameraProjectionMatrix(camera, nearPlane, farPlane);
        Matrix view = GetCameraMatrix(camera);
        rlSetMatrixProjection(proj);

        DrawGrid(8, 0.5f);
        DrawCube({ 0, 0.25, 0 }, 0.5, 0.5, 0.5, BLUE);
        DrawCubeWires({ 0, 0.25, 0 }, 0.5, 0.5, 0.5, DARKBLUE);
        EndMode3D();

        DrawCircle(mousePos.x, mousePos.y, 15.0f, GREEN);
        EndTextureMode();

        BeginMode3D(mainCamera);
        DrawGrid(8, 0.5f);
        DrawCube({ 0, 0.25, 0 }, 0.5, 0.5, 0.5, BLUE);
        DrawCubeWires({ 0, 0.25, 0 }, 0.5, 0.5, 0.5, DARKBLUE);

        Vector3 ndc = { 2*mousePos.x/800-1, -(2*mousePos.y/800-1), 1 };
        Line3D line = GetCameraWorldRay(ndc, view, proj);
        DrawLine3D(line.start, line.end, GREEN);
        
        DrawCameraFrustrum(mainCamera, camera, nearPlane, farPlane);
        EndMode3D();

        // GUI pass.
        Texture2D texture = renderTexture.texture;
        DrawRectangle(500, 500, 275, 275, BLACK);
        DrawTexturePro(texture, { 0, 0, (float)texture.width, -(float)texture.height }, 
            { 500, 500, 275, 275 }, {}, 0, WHITE);
        DrawRectangleLines(500, 500, 275, 275, DARKGRAY);

        if (GuiButton({ 300, 10, 100, 20 }, TextFormat(orthoGraphic ? "Orthographic": "Perspective"))) {
            orthoGraphic = !orthoGraphic;
            camera.projection = orthoGraphic ? CAMERA_ORTHOGRAPHIC: CAMERA_PERSPECTIVE;
        }
        GuiSlider({ 50, 10, 200, 10 }, "Near", TextFormat("%3.2f", nearPlane), &nearPlane, 0.01f, 10.0f);
        GuiSlider({ 50, 25, 200, 10 }, "Far", TextFormat("%3.2f", farPlane), &farPlane, 0.01f, 10.0f);
        GuiSlider({ 50, 40, 200, 10 }, 
            TextFormat(orthoGraphic ? "Height": "FOV y"), 
            TextFormat("%3.2f", orthoGraphic? extents: fovy),
            orthoGraphic ? &extents: &fovy, 0.01f, orthoGraphic ? 10.0f : 179.9f);
        EndDrawing();
    }
}