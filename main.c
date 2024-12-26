/*******************************************************************************************
 *
 *   raylib [core] example - Initialize 3d camera free
 *
 *   Example originally created with raylib 1.3, last time updated with raylib 1.3
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define GRAVITY 0.5f
#define JUMP_FORCE 10.0f
#define COOLDOWN 3 // seconds

#define SOUNDTRACK_PATH "assets/sounds/soundtrack2.mp3"
#define SHOOT_SOUND_PATH "assets/sounds/shoot.wav"

#define LIST_INIT_CAP 256
#define da_append(da, item)                                                            \
    do                                                                                 \
    {                                                                                  \
        if ((da)->count == (da)->capacity)                                             \
        {                                                                              \
            (da)->capacity = (da)->capacity == 0 ? LIST_INIT_CAP : (da)->capacity * 2; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL);                                               \
        }                                                                              \
                                                                                       \
        (da)->items[(da)->count++] = (item);                                           \
    } while (0)

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    float vel_x;
    float vel_y;
    Texture2D texture;
    int lifes;
} Player;

typedef struct
{
    int x;
    int y;
    float size;
    float velocity;
    int is_released;
} Bullet;

typedef struct
{
    int x;
    int y;
    float size;
    float velocity;
    int is_released;
} Enemy;

typedef struct
{
    Bullet *items;
    int count;
    int capacity;
} Bullets;

typedef struct
{
    Enemy *items;
    int count;
    int capacity;
} Enemies;

enum ScreenTitle
{
    START,
    CONFIG,
    GAME,
    GAME_OVER,
    GAME_WON,
    GAME_LOST,
    GAME_RUNNING,
    GAME_FINISHED
};

typedef struct
{
    int name;
    int width;
    int height;
    int is_paused;
    int is_game_over;
    int is_game_started;
    int is_game_won;
    int is_game_lost;
    int is_game_running;
    int is_game_finished;
} Screen;

typedef struct
{
    Music music;
    Sound fire_sound;
    Texture2D bgTexture;
    Texture2D playerTexture;
    Texture2D bulletTexture;
    Texture2D artValTexture;
    Texture2D muteUnmuteTexture;
    Texture2D settingsTexture;
    Font font;
    Bullets bullets;
    Enemies enemies;
    Player player;
    Screen screen;
    int score;
    bool is_muted;
} Game;

typedef struct 
{
    Rectangle rect;
    Color color;
} Button;


int is_in_ground(int y, int height)
{
    return y >= height;
}

void LoadAudio(Game *game)
{
    InitAudioDevice();
    Music music = LoadMusicStream(SOUNDTRACK_PATH);
    game->music = music;
    SetMusicVolume(music, 0.1f);
    Sound fire_sound = LoadSound(SHOOT_SOUND_PATH);
    game->fire_sound = fire_sound;
    SetSoundVolume(fire_sound, 0.1f);
}

void SetActiveScreen(Game *game, int screen)
{
    game->screen.name = screen;
}

void DrawTextCenter(char *text, int fontSize, Color color)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int text_width = MeasureText(text, fontSize) / 2;
    DrawText(text, width / 2 - text_width, height / 2, fontSize, color);
}


void DrawButton(char *text, int x, int y, int width, int height, Color color, Color textColor)
{
    DrawRectangle(x, y, width, height, color);
    int text_width = MeasureText(text, 20) / 2;
    int text_height = MeasureTextEx(GetFontDefault(), text, 20, 1).y / 2;
    DrawText(text, x + (width/2) - text_width, y + height/2 - text_height, 20, textColor);
}

void startGame(Game *game) {
    game->screen.is_game_started = 1;
    game->screen.name = GAME;
};


void DrawLifes(Game *game)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int text_width = MeasureText("VIDAS", 20);
    DrawText("VIDAS", width - text_width - 50, 10, 20, WHITE);
    DrawText(TextFormat("%d", game->player.lifes), width - text_width + 35, 10, 20, GREEN);
}

void DrawScore(Game *game)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int text_width = MeasureText("PONTOS", 20);
    DrawText("PONTOS", 0 + text_width - 50, 10, 20, WHITE);
    DrawText(TextFormat("%d", game->score), 0 + text_width + 50, 10, 20, GREEN);
}

void DrawMuted(Game *game)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    Rectangle source = (Rectangle){game->muteUnmuteTexture.width/2, 0, game->muteUnmuteTexture.width/2, game->muteUnmuteTexture.height};
    Rectangle dest = (Rectangle){0, 0, game->muteUnmuteTexture.width/8, game->muteUnmuteTexture.height/4};
    DrawTexturePro(
        game->muteUnmuteTexture,
        source,
        dest,
        (Vector2){0,0},
        0,
        WHITE
    );
    if(CheckCollisionPointRec(GetMousePosition(), dest)) {
        DrawTexturePro(
            game->muteUnmuteTexture,
            source,
            dest,
            (Vector2){0, 0},
            0,
            LIGHTGRAY);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            game->is_muted = !game->is_muted;
            game->is_muted ? SetMusicVolume(game->music, 0.0f) : SetMusicVolume(game->music, 0.1f);
        }
    }
}

void DrawUnMuted(Game *game) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    Rectangle source = (Rectangle){0, 0, game->muteUnmuteTexture.width/2, game->muteUnmuteTexture.height};
    Rectangle dest = (Rectangle){0, 0, game->muteUnmuteTexture.width/8, game->muteUnmuteTexture.height/4};
    DrawTexturePro(
        game->muteUnmuteTexture,
        source,
        dest,
        (Vector2){0,0},
        0,
        WHITE
    );
    if(CheckCollisionPointRec(GetMousePosition(), dest)) {
        DrawTexturePro(
            game->muteUnmuteTexture,
            source,
            dest,
            (Vector2){0, 0},
            0,
            LIGHTGRAY);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            game->is_muted = !game->is_muted;
            game->is_muted ? SetMusicVolume(game->music, 0.0f) : SetMusicVolume(game->music, 0.1f);
        }
    }
}

void DrawSettingsIcon(Game *game) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int margin = 15;

    DrawTextureEx(game->settingsTexture, (Vector2){width - game->settingsTexture.width*0.15 - margin , 0 + 7}, 0,0.15, WHITE);
    Rectangle dest = (Rectangle){width - game->settingsTexture.width*0.15 - margin ,7,game->settingsTexture.width*0.15, game->settingsTexture.height*0.15};
    if(CheckCollisionPointRec(GetMousePosition(), dest)) {
        DrawTextureEx(game->settingsTexture, (Vector2){width - game->settingsTexture.width*0.15 - margin , 0 + 7}, 0,0.15, LIGHTGRAY);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // SetActiveScreen(game, CONFIG);
        }
    }
}


void DrawPausedScreen(Game *game)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    DrawText("PAUSADO", width / 2 - MeasureTextEx(GetFontDefault(), "PAUSADO", GetFontDefault().baseSize, 1).x, height / 2, 20, WHITE);
    game->is_muted ? DrawMuted(game) : DrawUnMuted(game);
    DrawSettingsIcon(game);
    EndDrawing();
}

void DrawMainScreen(Game *game)
{
    int width = game->screen.width;
    int height = game->screen.height;
    Vector2 mommy_text_size = MeasureTextEx(game->font, "MOMMY", 90, 1);
    int mommy_text_width = mommy_text_size.x / 2;
    int mommy_text_height = mommy_text_size.y / 2;

    Vector2 invaders_text_size = MeasureTextEx(game->font, "I SAID", 45, 1);
    int invaders_text_width = invaders_text_size.x / 2;
    int invaders_text_height = invaders_text_size.y / 2;
    DrawTexture(game->artValTexture, width / 2 - (game->artValTexture.width/2) , 0, WHITE);


    DrawTextEx(game->font, "MOMMY", (Vector2){width / 2 - mommy_text_width + (game->artValTexture.width/2) + 30, (height / 2) - mommy_text_height - (0.1*height)}, 90, 1, YELLOW);
    DrawTextEx(game->font, "I SAID", (Vector2){width / 2 - invaders_text_width + (game->artValTexture.width/2) + 30, (height / 2) - invaders_text_height}, 45, 1, YELLOW);

    // DrawText("INVADERS", width / 2 - (MeasureText("INVADERS", 50) / 2) + (game->artValTexture.width/2), (height / 2) - text_height, 50, GREEN);
    DrawButton("JOGAR", (0.05*width), (0.20*height) , 200, 50, YELLOW, BLACK);
    DrawButton("CONTROLES", (0.05*width), (0.35*height)  , 200, 50, YELLOW, BLACK);
    DrawButton("CRÉDITOS", (0.05*width), (0.50*height) , 200, 50, YELLOW, BLACK);

    bool mouseOverStartButton = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(0.05*width), (0.20*height), 200, 50});
    if(mouseOverStartButton) {
        DrawRectangleLines((0.05*width), (0.20*height), 200, 50, WHITE);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            startGame(game);
        }
    }

    bool mouseOverControllBtn = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(0.05*width), (0.35*height), 200, 50});
    if(mouseOverControllBtn) {
        DrawRectangleLines((0.05*width), (0.35*height), 200, 50, WHITE);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            SetActiveScreen(game, CONFIG);
        }
    }

    bool mouseOverCreditBtn = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(0.05*width), (0.50*height), 200, 50});
    if(mouseOverCreditBtn) {
        DrawRectangleLines((0.05*width), (0.50*height), 200, 50, WHITE);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            SetActiveScreen(game, CONFIG);
        }
    }
}

void DrawScoreMarkerMessage(Game*game) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    switch(game->score) {
        case 10:
            DrawTextCenter("Seu sobrenome é coragem!", 20, WHITE);
            break;
        case 20:
            DrawTextCenter("Viva a ucrânia", 20, WHITE);
            break;
        case 30:
            DrawTextCenter("Doa a quem doer!", 20, WHITE);
            break;
        case 40:
            DrawTextCenter("Sou mais forte!", 20, WHITE);
            break;
        default:
            DrawText("", width / 2, height / 2, 20, WHITE);
    }
}


Game InitGame(int width, int height)
{
    Game game = {0};
    game.screen.width = width;
    game.screen.height = height;
    game.screen.is_paused = 0;
    game.screen.is_game_over = 0;
    game.screen.is_game_started = 0;
    game.screen.is_game_won = 0;
    game.screen.is_game_lost = 0;
    game.screen.is_game_running = 0;
    game.screen.is_game_finished = 0;
    game.screen.name = START;

    Player player = {0};
    player.width = width * 0.1;
    player.height = height * 0.1;
    player.x = width / 2;
    player.y = height / 2;

    game.player = player;
    game.score = 0;
    return game;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int width = 960;
    const int height =  572;

    float ball_vel = -0.5f;
    int ball_x = width / 2;
    int ball_y = height / 2;
    const int ball_radius = 20;
    int paused = 0;

    Game game = InitGame(width, height);

    InitWindow(width, height, "MAMÃE INVADERS");

    SetTargetFPS(60);

    LoadAudio(&game);

    PlayMusicStream(game.music);



    Texture2D playerTexture = LoadTexture("assets/images/ship1.png");
    Texture2D bulletTexture = LoadTexture("assets/images/bullet.png");
    Texture2D bgTexture = LoadTexture("assets/images/background2.png");
    Texture2D artValTexture = LoadTexture("assets/images/artval.png");
    Texture2D muteUnmuteTexture = LoadTexture("assets/images/mute-unmute.png");
    Texture2D settingsTexture = LoadTexture("assets/images/settings.png");
    Font font = LoadFont("assets/space_invaders.ttf");

    Game *g = &game;
    g->artValTexture = artValTexture;
    g->muteUnmuteTexture = muteUnmuteTexture;
    g->settingsTexture = settingsTexture;
    g->font = font;

    Bullets bullets = {0};
    Enemies enemies = {0};
    g->bullets = bullets;
    g->enemies = enemies;

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateMusicStream(game.music);
        BeginDrawing();




        Rectangle source = {0, 0, (float)bgTexture.width, (float)bgTexture.height};
        Rectangle dest = {0, 0, (float)width, (float)height};
        DrawTexturePro(
            bgTexture,
            source,
            dest,
            (Vector2){0, 0},
            0,
            WHITE);

        if (game.screen.name == START)
        {
            DrawMainScreen(&game);
        }
        if (game.screen.is_game_started == 1)
        {

            if (IsKeyPressed(KEY_P))
            {
                paused = !paused;
                if (paused)
                {
                    PauseMusicStream(game.music);
                }
                else 
                {
                    ResumeMusicStream(game.music);
                }
            }

            if (paused)
            {
                DrawPausedScreen(&game);
                continue;
            }

            int ball_bottom = ball_y + playerTexture.height * 10;
            if (ball_bottom >= height)
            {
                ball_vel = 0;
                ball_y = height - playerTexture.height * 10;
            }

            printf("Bullets: %d\n", bullets.count);

            // if (IsKeyPressed(KEY_SPACE) && is_in_ground(ball_bottom, height))
            // {
            //     ball_vel = -JUMP_FORCE;
            // }

            if (IsKeyDown(KEY_D))
            {
                ball_x += 10.2f;
                if ((ball_x + playerTexture.width * 10.0f)> GetScreenWidth())
                {
                    ball_x = GetScreenWidth() - playerTexture.width * 10.0f;
                }
            }

            if (IsKeyDown(KEY_A))
            {
                ball_x -= 10.2f;
                if (ball_x < 0)
                {
                    ball_x = 0;
                }
            }

            if (IsKeyPressed(KEY_O))
            {
                Bullet bullet = {ball_x + 10, ball_y, 10, 10, 1};
                da_append(&bullets, bullet);
                PlaySound(game.fire_sound);

                da_append(&enemies, ((Enemy){rand() % width, 100, 10, 3.0f, 1}));
            }

            ball_y += ball_vel;
            ball_vel += 0.5f;
            ClearBackground(WHITE);
            Rectangle source = {
                0,
                0,
                (float)playerTexture.width,
                (float)playerTexture.height};
            DrawTextureEx(
                playerTexture,
                (Vector2){ball_x, ball_y},
                0,
                10.0f,
                WHITE);
            DrawScoreMarkerMessage(&game);
            DrawScore(&game);
            DrawLifes(&game);
            for (int i = 0; i < bullets.count; i++)
            {
                if (bullets.items[i].is_released)
                {
                    bullets.items[i].y -= bullets.items[i].velocity;
                    DrawTextureEx(
                        bulletTexture,
                        (Vector2){bullets.items[i].x, bullets.items[i].y},
                        0,
                        10.0f,
                        WHITE);
                    if (bullets.items[i].y < 0 || bullets.items[i].x > width || bullets.items[i].x < 0)
                    {
                        bullets.items = realloc(bullets.items, (bullets.count + 20) * sizeof(Bullet));
                    }

                    for (int j = 0; j < enemies.count; j++)
                    {
                        Rectangle enemy = {enemies.items[j].x, enemies.items[j].y, 150, 50};
                        Rectangle bullet = {bullets.items[i].x, bullets.items[i].y, bulletTexture.width * 10.0f, bulletTexture.height * 10.0f};
                        if (CheckCollisionRecs(enemy, bullet) && enemies.items[j].is_released)
                        {
                            enemies.items[j].is_released = 0;
                            bullets.items[i].is_released = 0;
                            Game *g = &game;
                            g->score += 1;
                        }
                    }
                }
            }

            for (int i = 0; i < enemies.count; i++)
            {
                if (enemies.items[i].is_released)
                {

                    enemies.items[i].x += enemies.items[i].velocity;

                    if (CheckCollisionRecs(
                            (Rectangle){ball_x, ball_y, playerTexture.width * 10.0f, playerTexture.height * 10.0f},
                            (Rectangle){enemies.items[i].x, enemies.items[i].y, 150, 50}))
                    {
                        enemies.items[i].is_released = 0;
                        Game *g = &game;
                        g->score -= 1;
                    }

                    DrawRectangle(enemies.items[i].x, enemies.items[i].y, 150, 50, RED);
                    if (i == 0)
                    {
                        DrawText("Máfia do transporte",
                                 enemies.items[i].x,
                                 enemies.items[i].y, 20, WHITE);
                    }
                    else if (i == 1)
                    {
                        DrawText("Cracolândia",
                                 enemies.items[i].x,
                                 enemies.items[i].y, 20, WHITE);
                    }
                    else if (i == 3)
                    {
                        DrawText("Gabriel Monteiro",
                                 enemies.items[i].x,
                                 enemies.items[i].y, 20, WHITE);
                    }
                    else
                    {
                        DrawText("Esquerdista",
                                 enemies.items[i].x,
                                 enemies.items[i].y, 20, WHITE);
                    }

                    if (enemies.items[i].x == 0)
                    {
                        enemies.items[i].y += 50;
                        enemies.items[i].velocity *= -1;
                    }

                    if (enemies.items[i].x >= width - 150)
                    {
                        enemies.items[i].y += 50;
                        enemies.items[i].velocity *= -1;
                    }
                }
            }
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    UnloadMusicStream(game.music);
    UnloadSound(game.fire_sound);
    UnloadTexture(bgTexture);
    CloseAudioDevice();
    free(bullets.items);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
