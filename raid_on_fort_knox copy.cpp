#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "sprites.h"

// pozycja pantery
float pantherX = 11.0f * CELL_SIZE + (CELL_SIZE - PANTHER_WIDTH) / 2.0f;
float pantherY = 6.0f * CELL_SIZE + (CELL_SIZE - PANTHER_HEIGHT) / 2.0f;

// Kierunek pantery w sensie pikseli (np. (1,0) to w prawo)
float pantherSpeed = 1.0f; // Prędkość ruchu w pikselach/klatkę
int pantherDirX = 0;       // Kierunek na osi X: 1 = w prawo, -1 = w lewo
int pantherDirY = 1;       // Kierunek na osi Y: 1 = w dół, -1 = w górę

// pozycja startu gracza
float PlayerPosXstart, PlayerPosYstart;

// czy pantera jest w trybie „bezpiecznym”?
bool pantherIsDisabled = false;

// licznik czasu, ile jeszcze pantera będzie wyłączona. coś nie bardzo działa i
// chyba nie jest używane
int pantherDisableTimer = 5;

// np. 180 klatek = ok. 3 sekundy (jeśli 60 FPS)
const int PANTHER_DISABLE_TIME = 180;

// Rozmiar okna graficznego
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

// Szybkość ruchu w pikselach na klatkę
float speed = 2.0f;

// ----------------- KOLORY ELEMENTÓW GRY---------------------------
const SDL_Color COLOR_WALL = {0, 0, 255, 255};     // Niebieski
const SDL_Color COLOR_PLAYER = {0, 255, 0, 255};   // Zielony
const SDL_Color COLOR_PATH = {0, 0, 0, 255};       // Czarny (tło)
const SDL_Color COLOR_LIVES = {255, 0, 0, 255};    // Czerwony
const SDL_Color COLOR_START = {255, 255, 0, 255};  // Żółty
const SDL_Color COLOR_GOLD = {255, 215, 0, 255};   // Złoty
const SDL_Color COLOR_GOLD2 = {255, 115, 66, 255}; // Złoto "podwójne"
const SDL_Color COLOR_PANTHER = {255, 0, 0, 255};  // Pantera
const SDL_Color COLOR_PANTHER_DISABLED = {255, 255, 11,
                                          255}; // Pantera pod kolizji

// ----------------- POZYCJA GRACZA Pozycja w pikselach (lewy górny
// róg)--------------------
float PlayerPosX, PlayerPosY;

// Pozycja docelowa w pikselach
float PlayerTargetPosX, PlayerTargetPosY;

// Czy obecnie animujemy ruch
bool isMoving = false;

// ----------------- POZYCJA PANTERY --------------------
bool justCollidedWithPanther = false;

// Kierunek w sensie kafelków (np. (1,0) to w prawo)
int dirCellX = 0, dirCellY = 0;

// SPRAWDZENIE KOLIZJI Z BOUNDING-BOXEM PANTERY
bool checkPantherBoxCollision(float x1, float y1, float w1, float h1, float x2,
                              float y2, float w2, float h2)
{
    // If one rectangle is on left side of other
    if (x1 + w1 <= x2)
        return false;
    if (x2 + w2 <= x1)
        return false;
    if (y1 + h1 <= y2)
        return false;
    if (y2 + h2 <= y1)
        return false;
    return true;
}

// SPRAWDZENIE KOLIZJI Z BOUNDING-BOXEM STARTU
bool checkStartBoxCollision(float x1, float y1, float w1, float h1, float x2,
                            float y2, float w2, float h2)
{
    // If one rectangle is on left side of other
    if (x1 + w1 <= x2)
        return false;
    if (x2 + w2 <= x1)
        return false;
    if (y1 + h1 <= y2)
        return false;
    if (y2 + h2 <= y1)
        return false;
    return true;
}

void drawPlayerSprite(SDL_Renderer *renderer, int x, int y)
{
    // Obliczamy skalę w pikselach
    int scaleX = PLAYER_WIDTH / SPRITE_WIDTH;   // 20 / 8 = 2
    int scaleY = PLAYER_HEIGHT / SPRITE_HEIGHT; // 20 / 8 = 2

    // Rysujemy sprite
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = playerSprite[row][col];

            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_PLAYER : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Rysujemy dokładnie dopasowany prostokąt
            SDL_Rect rect;
            rect.x = x + col * scaleX;
            rect.y = y + row * scaleY;
            rect.w = scaleX;
            rect.h = scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// rysowanie gracza w prawo
void drawPlayerSpriteRight(SDL_Renderer *renderer, int x, int y)
{
    // Obliczamy skalę w pikselach
    int scaleX = PLAYER_WIDTH / SPRITE_WIDTH;   // 20 / 8 = 2
    int scaleY = PLAYER_HEIGHT / SPRITE_HEIGHT; // 20 / 8 = 2

    // Rysujemy sprite
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = playerSpriteRight[row][col];

            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_PLAYER : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Rysujemy dokładnie dopasowany prostokąt
            SDL_Rect rect;
            rect.x = x + col * scaleX;
            rect.y = y + row * scaleY;
            rect.w = scaleX;
            rect.h = scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// rysowanie gracza w dół
void drawPlayerSpriteDown(SDL_Renderer *renderer, int x, int y)
{
    // Obliczamy skalę w pikselach
    int scaleX = PLAYER_WIDTH / SPRITE_WIDTH;   // 20 / 8 = 2
    int scaleY = PLAYER_HEIGHT / SPRITE_HEIGHT; // 20 / 8 = 2

    // Rysujemy sprite
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = playerSpriteDown[row][col];

            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_PLAYER : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Rysujemy dokładnie dopasowany prostokąt
            SDL_Rect rect;
            rect.x = x + col * scaleX;
            rect.y = y + row * scaleY;
            rect.w = scaleX;
            rect.h = scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// rysowanie gracza w lewo
void drawPlayerSpriteLeft(SDL_Renderer *renderer, int x, int y)
{
    // Obliczamy skalę w pikselach
    int scaleX = PLAYER_WIDTH / SPRITE_WIDTH;   // 20 / 8 = 2
    int scaleY = PLAYER_HEIGHT / SPRITE_HEIGHT; // 20 / 8 = 2

    // Rysujemy sprite
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = playerSpriteLeft[row][col];

            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_PLAYER : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Rysujemy dokładnie dopasowany prostokąt
            SDL_Rect rect;
            rect.x = x + col * scaleX;
            rect.y = y + row * scaleY;
            rect.w = scaleX;
            rect.h = scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art ZŁOTA w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawGoldSprite(SDL_Renderer *renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)GOLD_WIDTH / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)GOLD_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5

    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = goldSprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_GOLD : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art PODWÓJNEGO ZŁOTA w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawGold2Sprite(SDL_Renderer *renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)GOLD2_WIDTH / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)GOLD2_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = gold2Sprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_GOLD2 : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art PANTERY w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawPantherSprite(SDL_Renderer *renderer, float x, float y,
                       bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH / (float)SPRITE_WIDTH; // 20 / 8 = 2.5
    float scaleY =
        (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c =
                pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER)
                        : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art PANTERY w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawPantherSpriteRight(SDL_Renderer *renderer, float x, float y,
                            bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH / (float)SPRITE_WIDTH; // 20 / 8 = 2.5
    float scaleY =
        (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteRight[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c =
                pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER)
                        : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art PANTERY w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawPantherSpriteLeft(SDL_Renderer *renderer, float x, float y,
                           bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH / (float)SPRITE_WIDTH; // 20 / 8 = 2.5
    float scaleY =
        (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteLeft[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c =
                pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER)
                        : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art PANTERY w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawPantherSpriteDown(SDL_Renderer *renderer, float x, float y,
                           bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH / (float)SPRITE_WIDTH; // 20 / 8 = 2.5
    float scaleY =
        (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteDown[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c =
                pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER)
                        : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Rysujemy pixel-art IKONA STARTU GRACZA w miejscu (x,y) o szerokości i
// wysokości docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawStartSprite(SDL_Renderer *renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)START_WIDTH / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)START_HEIGHT / (float)SPRITE_HEIGHT; // 20 / 8 = 2.5
    for (int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for (int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = startSprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_START : COLOR_PATH;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            // Rysujemy kwadracik scaleX × scaleY
            float drawX = x + col * scaleX;
            float drawY = y + row * scaleY;
            SDL_Rect rect;
            rect.x = (int)drawX;
            rect.y = (int)drawY;
            rect.w = (int)scaleX;
            rect.h = (int)scaleY;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// (Opcjonalna) funkcja kolizji z rogami bounding-boxa
bool checkCollisionWithWalls(float newX, float newY,
                             const std::vector<std::vector<int>> &maze)
{
    float left = newX;
    float top = newY;
    float right = newX + PLAYER_WIDTH - 1;
    float bottom = newY + PLAYER_HEIGHT - 1;

    auto isWallAtPixel = [&](float px, float py)
    {
        if (px < 0 || py < 0)
            return true; // poza mapą
        int tileX = (int)(px / CELL_SIZE);
        int tileY = (int)(py / CELL_SIZE);
        if (tileY < 0 || tileY >= (int)maze.size() || tileX < 0 ||
            tileX >= (int)maze[tileY].size())
        {
            return true; // poza tablicą
        }
        return (maze[tileY][tileX] == 1);
    };

    if (isWallAtPixel(left, top))
        return true;
    if (isWallAtPixel(right, top))
        return true;
    if (isWallAtPixel(left, bottom))
        return true;
    if (isWallAtPixel(right, bottom))
        return true;

    return false;
}

// (Opcjonalna) funkcja kolizji z rogami bounding-boxa pantery
bool checkPantherCollisionWithWalls(float newX, float newY,
                                    const std::vector<std::vector<int>> &maze)
{
    float left = newX;
    float top = newY;
    float right = newX + PANTHER_WIDTH - 1;
    float bottom = newY + PANTHER_HEIGHT - 1;

    auto isWallAtPixel = [&](float px, float py)
    {
        if (px < 0 || py < 0)
            return true; // poza mapą
        int tileX = (int)(px / CELL_SIZE);
        int tileY = (int)(py / CELL_SIZE);
        if (tileY < 0 || tileY >= (int)maze.size() || tileX < 0 ||
            tileX >= (int)maze[tileY].size())
        {
            return true; // poza tablicą
        }
        return (maze[tileY][tileX] == 1);
    };

    if (isWallAtPixel(left, top))
        return true;
    if (isWallAtPixel(right, top))
        return true;
    if (isWallAtPixel(left, bottom))
        return true;
    if (isWallAtPixel(right, bottom))
        return true;

    return false;
}

// Funkcja do obliczenia, dokąd można dojść w danym kierunku (dx, dy) aż do
// ściany.
void computeTargetCell(const std::vector<std::vector<int>> &maze,
                       int startCellX, int startCellY, int dx, int dy,
                       int &outX, int &outY)
{
    int newX = startCellX;
    int newY = startCellY;

    while (true)
    {
        int testX = newX + dx;
        int testY = newY + dy;

        if (testY < 0 || testY >= (int)maze.size() || testX < 0 ||
            testX >= (int)maze[testY].size())
        {
            break; // poza tablicą
        }
        if (maze[testY][testX] == 1)
        {
            break; // ściana
        }
        newX = testX;
        newY = testY;
    }
    outX = newX;
    outY = newY;
}

// Funkcja do rysowania prostokąta - używana przy ścianach, złocie, itp.
void drawRect(SDL_Renderer *renderer, int x, int y, int w, int h,
              SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

// Funkcja do rysowania tekstu
void renderText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text,
                int x, int y, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface)
    {
        std::cerr << "Failed to create text surface: " << TTF_GetError()
                  << std::endl;
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        std::cerr << "Failed to create text texture: " << TTF_GetError()
                  << std::endl;
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// GŁÓWNA PĘTLA PROGRAMU-----------------------------------------------------
int main(int argc, char *argv[])
{

    // Licznik punktów, flaga złota
    int score = 0;
    bool hasGold = false;

    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError()
                  << std::endl;
        return 1;
    }
    // Inicjalizacja SDL_ttf
    if (TTF_Init() == -1)
    {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError()
                  << std::endl;
        SDL_Quit();
        return 1;
    }
    // Wczytanie czcionki
    TTF_Font *font =
        TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 21);
    if (!font)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Okno
    SDL_Window *window = SDL_CreateWindow(
        "Raid on Fort Knox - Pixel sprite", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Renderer
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer creation failed: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Startowa pozycja (wycentrowana w kafelku 19,1).
    PlayerPosX = 19.0f * CELL_SIZE + (CELL_SIZE - PLAYER_WIDTH) / 2.0f;
    PlayerPosY = 1.0f * CELL_SIZE + (CELL_SIZE - PLAYER_HEIGHT) / 2.0f;

    PlayerPosXstart = 19.0f * CELL_SIZE + (CELL_SIZE - GOLD_WIDTH) / 2.0f;
    PlayerPosYstart = 0.0f * CELL_SIZE + (CELL_SIZE - GOLD_HEIGHT) / 2.0f;

    // Początkowo stoimy w miejscu
    PlayerTargetPosX = PlayerPosX;
    PlayerTargetPosY = PlayerPosY;

    bool running = true;
    SDL_Event event;

    while (running)
    {
        // 1) Obsługa zdarzeń
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                // Określamy aktualną komórkę (środek gracza)
                int cellX = (int)((PlayerPosX + PLAYER_WIDTH / 2) / CELL_SIZE);
                int cellY = (int)((PlayerPosY + PLAYER_HEIGHT / 2) / CELL_SIZE);

                if (event.key.keysym.sym == SDLK_UP ||
                    event.key.keysym.sym == SDLK_DOWN)
                {
                    // Snapujemy w osi X (wyśrodkowujemy)
                    float centerX = cellX * CELL_SIZE + CELL_SIZE / 2.0f;
                    PlayerPosX = centerX - PLAYER_WIDTH / 2.0f;

                    // Kierunek
                    dirCellX = 0;
                    dirCellY = (event.key.keysym.sym == SDLK_UP) ? -1 : 1;
                }
                else if (event.key.keysym.sym == SDLK_LEFT ||
                         event.key.keysym.sym == SDLK_RIGHT)
                {
                    // Snapujemy w osi Y
                    float centerY = cellY * CELL_SIZE + CELL_SIZE / 2.0f;
                    PlayerPosY = centerY - PLAYER_HEIGHT / 2.0f;

                    dirCellY = 0;
                    dirCellX = (event.key.keysym.sym == SDLK_LEFT) ? -1 : 1;
                }
                else
                {
                    // np. spacja, ESC, itp. - ignorujemy
                    continue;
                }

                // Teraz obliczamy docelową komórkę
                cellX = (int)((PlayerPosX + PLAYER_WIDTH / 2) / CELL_SIZE);
                cellY = (int)((PlayerPosY + PLAYER_HEIGHT / 2) / CELL_SIZE);

                int tCellX, tCellY;
                computeTargetCell(maze, cellX, cellY, dirCellX, dirCellY,
                                  tCellX, tCellY);

                // Zamieniamy na piksele (też wycentrowane)
                PlayerTargetPosX =
                    tCellX * CELL_SIZE + (CELL_SIZE - PLAYER_WIDTH) / 2.0f;
                PlayerTargetPosY =
                    tCellY * CELL_SIZE + (CELL_SIZE - PLAYER_HEIGHT) / 2.0f;

                isMoving = true;
            }
        }

        // 2) Logika płynnego ruchu
        if (isMoving)
        {
            float dx = PlayerTargetPosX - PlayerPosX;
            float dy = PlayerTargetPosY - PlayerPosY;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > speed)
            {
                float stepX = PlayerPosX + speed * (dx / dist);
                float stepY = PlayerPosY + speed * (dy / dist);

                // Sprawdzamy kolizję - wąski korytarz, raczej i tak jej nie
                // będzie, ale zostawiamy "na wszelki wypadek".
                if (!checkCollisionWithWalls(stepX, stepY, maze))
                {
                    PlayerPosX = stepX;
                    PlayerPosY = stepY;
                }
                else
                {
                    // Kolizja - zatrzymujemy ruch
                    isMoving = false;
                }
            }
            else
            {
                // Ostatni krok
                float stepX = PlayerTargetPosX;
                float stepY = PlayerTargetPosY;

                if (!checkCollisionWithWalls(stepX, stepY, maze))
                {
                    PlayerPosX = stepX;
                    PlayerPosY = stepY;
                }
                isMoving = false;
            }
            if (pantherIsDisabled)
            {
                pantherDisableTimer--;
                if (pantherDisableTimer <= 0)
                {
                    pantherIsDisabled = false;
                    std::cout << "[DEBUG] Pantera znowu niebezpieczna.\n";
                }
            }
        }

        // 3) Sprawdzamy, czy zbieramy złoto / wejście na start
        {
            int checkCellX = (int)((PlayerPosX + PLAYER_WIDTH / 2) / CELL_SIZE);
            int checkCellY =
                (int)((PlayerPosY + PLAYER_HEIGHT / 2) / CELL_SIZE);

            if (checkCellY >= 0 && checkCellY < (int)maze.size() &&
                checkCellX >= 0 && checkCellX < (int)maze[checkCellY].size())
            {
                int cellValue = maze[checkCellY][checkCellX];
                if (cellValue == 4)
                {
                    if (hasGold)
                    {
                        isMoving = false;
                    }
                    else if (!hasGold)
                    {
                        std::cout << "Zbierasz zloto pojedyncze\n";
                        maze[checkCellY][checkCellX] = 0;
                        hasGold = true;
                        if (hasGold)
                        {
                            isMoving = false;
                        }
                    }
                }
                else if (cellValue == 3)
                {
                    if (hasGold)
                    {
                        isMoving = false;
                    }
                    else if (!hasGold)
                    {
                        std::cout << "Zbierasz zloto podwojne\n";
                        maze[checkCellY][checkCellX] = 4;
                        hasGold = true;
                        if (hasGold)
                        {
                            isMoving = false;
                        }
                    }
                }
                // kolizja ze startem
                if (checkStartBoxCollision(PlayerPosX, PlayerPosY, PLAYER_WIDTH,
                                           PLAYER_HEIGHT, PlayerPosXstart,
                                           PlayerPosYstart, START_WIDTH,
                                           START_HEIGHT))
                {
                    if (hasGold)
                    {
                        std::cout << "Oddajesz zloto na start!\n";
                        score++;
                        hasGold = false;
                    }
                    else
                    {
                        std::cout << "Nie masz złota!\n";
                    }
                }
                else if (cellValue == 5)
                {
                    // Tu np. mechanika życia
                }

                if (!pantherIsDisabled)
                {
                    // Kolizja z panterą
                    if (checkPantherBoxCollision(
                            PlayerPosX, PlayerPosY, PLAYER_WIDTH, PLAYER_HEIGHT,
                            pantherX, pantherY, PANTHER_WIDTH,
                            PANTHER_HEIGHT))
                    {
                        if (!justCollidedWithPanther)
                        {
                            if (hasGold)
                            {
                                justCollidedWithPanther = true;
                                pantherIsDisabled = true;
                                pantherDisableTimer = PANTHER_DISABLE_TIME;
                                std::cout
                                    << "Złapała Cię pantera. Tracisz złoto!\n";
                                hasGold = false;
                            }
                            else
                            {
                                std::cout
                                    << "Złapała Cię pantera. Tracisz życie!\n";
                                // Usuwasz jedno życie z labiryntu lub
                                // zmniejszasz licznik "lives"
                                maze[0][2] = 0;
                            }
                        }
                        else
                        {
                            // tu "else" jest wobec warunku cellValue == 7
                            // tzn. jeżeli to *nie* pantera, to znaczy, że gracz
                            // stoi gdzieś indziej i można zresetować
                            // justCollidedWithPanther
                            justCollidedWithPanther = false;
                        }
                    }
                    /////////////////////////// ruszanie pantery
                    // Przesunięcie pantery w bieżącym kierunku
                    pantherX += pantherSpeed * pantherDirX; // Ruch w poziomie
                    pantherY += pantherSpeed * pantherDirY; // Ruch w pionie

                    /* // Sprawdzanie kolizji z korytarzem (ścianami)
                     int cellX = (int)(pantherX / CELL_SIZE); // Pozycja w
                 siatce int cellY = (int)(pantherY / CELL_SIZE);

                     // Jeśli pantera napotka ścianę, zmień kierunek
                     if (maze[cellY][cellX] == 1) { // 1 = ściana
                     if (pantherDirX != 0) { // Porusza się w poziomie
                     pantherDirX *= -1; // Zmiana kierunku na osi X
                     } else if (pantherDirY != 0) { // Porusza się w pionie
                     pantherDirY *= -1; // Zmiana kierunku na osi Y
                     }
                 }
                     */
                    ////////////////////////////////////////// koniec ruszanie pantery

                    // Jeśli pantera napotka ścianę, zmień kierunek
                    if (checkPantherCollisionWithWalls(pantherX, pantherY,
                                                       maze))
                    {
                        if (pantherDirX != 0)
                        {                      // Porusza się w poziomie
                            pantherDirX *= -1; // Zmiana kierunku na osi X
                            pantherX +=
                                pantherSpeed * pantherDirX; // Cofnięcie o krok
                        }
                        else if (pantherDirY != 0)
                        {                      // Porusza się w pionie
                            pantherDirY *= -1; // Zmiana kierunku na osi Y
                            pantherY +=
                                pantherSpeed * pantherDirY; // Cofnięcie o krok
                        }
                    }
                }
            }
        }
        // 4) Renderowanie
        SDL_SetRenderDrawColor(renderer, COLOR_PATH.r, COLOR_PATH.g,
                               COLOR_PATH.b, COLOR_PATH.a);
        SDL_RenderClear(renderer);

        // Rysowanie labiryntu
        for (size_t y = 0; y < maze.size(); ++y)
        {
            for (size_t x = 0; x < maze[y].size(); ++x)
            {
                int val = maze[y][x];
                if (val == 1)
                {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE,
                             CELL_SIZE, COLOR_WALL);
                }
                else if (val == 4)
                {
                    drawGoldSprite(renderer, x * CELL_SIZE, y * CELL_SIZE);
                }
                else if (val == 3)
                {
                    drawGold2Sprite(renderer, x * CELL_SIZE, y * CELL_SIZE);
                }
                else if (val == 5)
                {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE,
                             CELL_SIZE, COLOR_LIVES);
                } // else if (val == 6) {
                  // drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE,
                  // CELL_SIZE, CELL_SIZE, COLOR_START);
                //}
                else
                {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE,
                             CELL_SIZE, COLOR_PATH);
                }
            }
        }

        // Rysowanie punktów
        std::string scoreText = "Punkty: " + std::to_string(score);
        renderText(renderer, font, scoreText, 5 * CELL_SIZE, 0,
                   {255, 255, 255, 255});

        // *** RYSOWANIE GRACZA: pixel-art sprite ***

        // sprawdzamy, czy gracz idzie w lewo, prawo, górę, dół
        if (!isMoving)
        {
            drawPlayerSprite(renderer, PlayerPosX, PlayerPosY);
        }
        else if (dirCellX == 1)
        {
            drawPlayerSpriteRight(renderer, PlayerPosX, PlayerPosY);
        }
        else if (dirCellX == -1)
        {
            drawPlayerSpriteLeft(renderer, PlayerPosX, PlayerPosY);
        }
        else if (dirCellY == 1)
        {
            drawPlayerSpriteDown(renderer, PlayerPosX, PlayerPosY);
        }
        else if (dirCellY == -1)
        {
            drawPlayerSprite(renderer, PlayerPosX, PlayerPosY);
            //} else {
            // drawPlayerSprite(renderer, PlayerPosX, PlayerPosY);
        }
        // Rysowanie pantery
        drawPantherSprite(renderer, pantherX, pantherY, pantherIsDisabled);

        drawStartSprite(renderer, 19 * CELL_SIZE, 0 * CELL_SIZE);
        // Wyświetlanie
        SDL_RenderPresent(renderer);

        // Krótka pauza (~25 FPS)
        SDL_Delay(10);
    }

    // Sprzątanie
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
