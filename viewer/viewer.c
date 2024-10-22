#include <raylib.h>
#include <raymath.h>

void DrawFPSEx(int x, int y, int font_size, Color color) {
    int fps = GetFPS();

    DrawText(TextFormat("FPS: %d", fps),
            x, y, font_size, color);
}

int main() {
    InitWindow(2000, 1000, "Simpc Viewer");

    SetTargetFPS(60);

    Vector2 circle_velocity = Vector2Zero();

    Vector2 circle_pos = {
        (float)GetScreenWidth() / 2,
        (float)GetScreenHeight() / 2
    };

    const float PUSH_BACK_RADIUS = 400;
    const float CIRCLE_RUNNING_SPEED = 100;

    while(!WindowShouldClose()) {
        BeginDrawing();
        {
            Vector2 mouse_pos = GetMousePosition();

            Vector2 vector_diff = Vector2Subtract(
                mouse_pos, circle_pos
            );

            
            float distance = Vector2Distance(circle_pos, mouse_pos);
            circle_velocity = Vector2Zero();

            if(distance <= PUSH_BACK_RADIUS) {
                Vector2 dir = Vector2Scale(vector_diff, -1.0f / distance);
                float closeness = 1.0f - (distance / PUSH_BACK_RADIUS);
                circle_velocity = Vector2Scale(dir, closeness * CIRCLE_RUNNING_SPEED);
            }

            ClearBackground(WHITE);

            DrawFPSEx(0, 0, 40, BLACK);

            DrawText(
                TextFormat("mouse pos: <%.2f, %.2f>",
                    mouse_pos.x, mouse_pos.y),
                0, 40, 40, BLACK
            );


            DrawCircle(
                circle_pos.x, circle_pos.y,
                200, RED
            );
            
            circle_pos =
                Vector2Add(circle_pos, circle_velocity);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
