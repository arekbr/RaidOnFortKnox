#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <iostream>
#include <cmath>    // std::sqrt
#include <string>
#include <cstdlib>
#include <ctime>

class MazeGenerator {
public:
    // Konstruktor
    MazeGenerator(const std::vector<std::vector<int>>& templateMaze)
        : mazeTemplate(templateMaze),
          rows(templateMaze.size()),
          cols(templateMaze[0].size()) {
        std::srand(std::time(nullptr)); // Ustawienie seed dla RNG
    }

    // Funkcja generująca pojedynczy labirynt
    std::vector<std::vector<int>> generateMaze() {
        std::vector<std::vector<int>> maze = mazeTemplate; // Kopia szablonu

        // Randomizowanie zawartości labiryntu (złoto i drogi)
        for (int i = 1; i < rows - 1; ++i) {
            for (int j = 1; j < cols - 1; ++j) {
                if (maze[i][j] != 1) { // Nie zmieniaj ścian
                    maze[i][j] = randomElement({0, 4, 3}); // Losuj drogę lub złoto
                }
            }
        }

        // Upewnienie się, że labirynt jest możliwy do przejścia
        ensureConnectivity(maze);

        return maze;
    }

private:
    const std::vector<std::vector<int>>& mazeTemplate;
    int rows;
    int cols;

    // Funkcja zwracająca losowy element z wektora
    int randomElement(const std::vector<int>& elements) {
        return elements[std::rand() % elements.size()];
    }

    // Funkcja zapewniająca, że labirynt jest możliwy do przejścia
    void ensureConnectivity(std::vector<std::vector<int>>& maze) {
        for (int i = 1; i < rows - 1; ++i) {
            for (int j = 1; j < cols - 1; ++j) {
                if (maze[i][j] == 0) {
                    bool isolated = true;
                    for (auto [dx, dy] : std::vector<std::pair<int, int>>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                        int ni = i + dx, nj = j + dy;
                        if (maze[ni][nj] != 1) {
                            isolated = false;
                            break;
                        }
                    }
                    if (isolated) {
                        maze[i][j] = 1; // Zamień izolowaną drogę na ścianę
                    }
                }
            }
        }
    }
};

// ----------------- USTAWIENIA ------------------------

// Rozmiar kafelka (tunel ma 1 kafelek szerokości)
const int CELL_SIZE = 25;


// Rozmiar "logiczne" sprajtów (używane do snapowania i kolizji bounding-box)
const int PLAYER_WIDTH      = 20;
const int PLAYER_HEIGHT     = 20;
const int START_WIDTH       = 20;
const int START_HEIGHT      = 20;
const int GOLD_WIDTH        = 20;
const int GOLD_HEIGHT       = 20;
const int GOLD2_WIDTH       = 20;
const int GOLD2_HEIGHT      = 20;
const int PANTHER_WIDTH     = 20;
const int PANTHER_HEIGHT    = 20;


// pozycja pantery
float pantherX = 11.0f * CELL_SIZE + (CELL_SIZE - PANTHER_WIDTH) / 2.0f;
float pantherY =  6.0f * CELL_SIZE + (CELL_SIZE - PANTHER_HEIGHT)/ 2.0f;    

// Kierunek pantery w sensie pikseli (np. (1,0) to w prawo)
float pantherSpeed = 1.0f; // Prędkość ruchu w pikselach/klatkę
int pantherDirX = 0;      // Kierunek na osi X: 1 = w prawo, -1 = w lewo
int pantherDirY = 1;      // Kierunek na osi Y: 1 = w dół, -1 = w górę



//pozycja startu
float posXstart, posYstart;

// czy pantera jest w trybie „bezpiecznym”?
bool pantherIsDisabled = false;      

// licznik czasu, ile jeszcze pantera będzie wyłączona. coś nie bardzo działa i chyba nie jest używane
int  pantherDisableTimer = 5;        

// np. 180 klatek = ok. 3 sekundy (jeśli 60 FPS)
const int PANTHER_DISABLE_TIME = 180; 

// Rozmiar okna graficznego
const int WINDOW_WIDTH  = 550;
const int WINDOW_HEIGHT = 600;

// Szybkość ruchu w pikselach na klatkę
float speed = 2.0f; 

// ----------------- KOLORY ELEMENTÓW GRY---------------------------
const SDL_Color COLOR_WALL              = { 0,   0, 255, 255 }; // Niebieski
const SDL_Color COLOR_PLAYER            = { 0, 255,   0, 255 }; // Zielony
const SDL_Color COLOR_PATH              = { 0,   0,   0, 255 }; // Czarny (tło)
const SDL_Color COLOR_LIVES             = { 255, 0,   0, 255 }; // Czerwony
const SDL_Color COLOR_START             = { 255, 255, 0, 255 }; // Żółty
const SDL_Color COLOR_GOLD              = { 255, 215, 0, 255 }; // Złoty
const SDL_Color COLOR_GOLD2             = { 255, 115, 66,255 }; // Złoto "podwójne"
const SDL_Color COLOR_PANTHER           = { 255, 0, 0, 255 }; // Pantera
const SDL_Color COLOR_PANTHER_DISABLED  = { 255, 255, 11, 255 }; // Pantera pod kolizji


// ----------------- POZYCJA GRACZA Pozycja w pikselach (lewy górny róg)--------------------
float posX, posY;          

// Pozycja docelowa w pikselach
float targetPosX, targetPosY;

// Czy obecnie animujemy ruch
bool  isMoving = false;

// ----------------- POZYCJA PANTERY --------------------
bool justCollidedWithPanther = false;

// Kierunek w sensie kafelków (np. (1,0) to w prawo)
int   dirCellX = 0, dirCellY = 0;



// SPRAWDZENIE KOLIZJI Z BOUNDING-BOXEM PANTERY
bool checkPantherBoxCollision(float x1, float y1, float w1, float h1,
                       float x2, float y2, float w2, float h2)
{
    // If one rectangle is on left side of other
    if (x1 + w1 <= x2) return false;
    if (x2 + w2 <= x1) return false;
    if (y1 + h1 <= y2) return false;
    if (y2 + h2 <= y1) return false;
    return true;
}

// SPRAWDZENIE KOLIZJI Z BOUNDING-BOXEM STARTU
bool checkStartBoxCollision(float x1, float y1, float w1, float h1,
                       float x2, float y2, float w2, float h2)
{
    // If one rectangle is on left side of other
    if (x1 + w1 <= x2) return false;
    if (x2 + w2 <= x1) return false;
    if (y1 + h1 <= y2) return false;
    if (y2 + h2 <= y1) return false;
    return true;
}

// ----------------- SPRITE GRACZA (pixel-art) ---------
// Rozmiar sprite'a (8x8) - to TYLKO do definicji tablicy
static const int SPRITE_WIDTH  = 8;
static const int SPRITE_HEIGHT = 8;
// kształt 8×8. 
// 1 = zapalony piksel, 0 = zgaszony (będzie kolorem tła).

static bool playerSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
    {0,0,1,1,1,1,0,0},
    {0,1,0,1,1,0,1,0},
    {0,0,1,1,1,1,0,0},
    {0,0,0,1,1,0,0,1},
    {0,1,1,1,1,1,1,0},
    {1,0,1,1,1,1,0,1},
    {0,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0}
};


static bool playerSpriteRight[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{1, 0, 1, 0, 0, 0, 0, 0},
{1, 1, 0, 1, 0, 0, 1, 0},
{0, 1, 1, 1, 0, 1, 0, 1},
{0, 1, 1, 1, 1, 1, 1, 1},
{0, 1, 1, 1, 1, 1, 1, 1},
{0, 1, 1, 1, 0, 1, 0, 1},
{0, 1, 0, 1, 0, 0, 1, 0},
{0, 1, 1, 0, 1, 0, 0, 0}
};

static bool playerSpriteDown[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{0, 0, 0, 0, 0, 0, 1, 1},
{1, 1, 1, 1, 1, 1, 1, 0},
{1, 0, 1, 1, 1, 1, 0, 1},
{0, 1, 1, 1, 1, 1, 1, 0},
{1, 0, 0, 1, 1, 0, 0, 0},
{0, 0, 1, 1, 1, 1, 0, 0},
{0, 1, 0, 1, 1, 0, 1, 0},
{0, 0, 1, 1, 1, 1, 0, 0}

};

static bool playerSpriteLeft[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{0, 0, 0, 1, 0, 1, 1, 0},
{0, 1, 0, 0, 1, 0, 1, 0},
{1, 0, 1, 0, 1, 1, 1, 0},
{1, 1, 1, 1, 1, 1, 1, 0},
{1, 1, 1, 1, 1, 1, 1, 0},
{1, 0, 1, 0, 1, 1, 1, 0},
{0, 1, 0, 0, 1, 0, 1, 1},
{0, 0, 0, 0, 0, 1, 0, 1}

};




// ----------------- SPRITE PANTERY (pixel-art) ---------
// KSZTAŁT 8×8.
static bool pantherSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
    {0,0,1,0,0,1,0,0}, 
    {0,0,1,1,1,1,0,0}, 
    {0,1,0,1,1,0,1,0}, 
    {1,1,1,1,1,1,1,1}, 
    {0,1,1,1,1,1,1,0}, 
    {0,1,1,1,1,1,1,0}, 
    {0,0,1,1,1,1,0,0}, 
    {0,0,0,0,1,0,0,0}   // tylne łapy
};

// ----------------- SPRITE PANTERY w prawo (pixel-art) ---------
// KSZTAŁT 8×8.
static bool pantherSpriteRight[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{0, 0, 0, 0, 1, 0, 0, 0}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{0, 1, 1, 1, 1, 0, 1, 1}, 
{0, 1, 1, 1, 1, 1, 1, 0}, 
{1, 1, 1, 1, 1, 1, 1, 0}, 
{0, 1, 1, 1, 1, 0, 1, 1}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{0, 0, 0, 0, 1, 0, 0, 0}
};


// ----------------- SPRITE PANTERY w lewo (pixel-art) ---------
// KSZTAŁT 8×8.
static bool pantherSpriteLeft[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{0, 0, 0, 1, 0, 0, 0, 0}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{1, 1, 0, 1, 1, 1, 1, 0}, 
{0, 1, 1, 1, 1, 1, 1, 1}, 
{0, 1, 1, 1, 1, 1, 1, 0}, 
{1, 1, 0, 1, 1, 1, 1, 0}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{0, 0, 0, 1, 0, 0, 0, 0}
};


// ----------------- SPRITE PANTERY w dół (pixel-art) ---------
// KSZTAŁT 8×8.
static bool pantherSpriteDown[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
{0, 0, 0, 0, 1, 0, 0, 0}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{0, 1, 1, 1, 1, 1, 1, 0}, 
{0, 1, 1, 1, 1, 1, 1, 0}, 
{1, 1, 1, 1, 1, 1, 1, 1}, 
{0, 1, 0, 1, 1, 0, 1, 0}, 
{0, 0, 1, 1, 1, 1, 0, 0}, 
{0, 0, 1, 0, 0, 1, 0, 0}

};

/*Funkcja obracanie(tablica):
    m = liczba_wierszy(tablica)
    n = liczba_kolumn(tablica)

    Stwórz nowa_tablica o wymiarach [n][m] (odwrócone wymiary)

    Dla każdego i w zakresie [0, m-1]:
        Dla każdego j w zakresie [0, n-1]:
            nowa_tablica[j][m-1-i] = tablica[i][j]

    Zwróć nowa_tablica
*/
    std::vector<std::vector<int>> obracanie(std::vector<std::vector<int>> maze)
    {
        int m = maze.size();
        int n = maze[0].size();
        std::vector<std::vector<int>> nowa_tablica(n, std::vector<int>(m));
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                nowa_tablica[j][m-1-i] = maze[i][j];
            }
        }
        return nowa_tablica;
    } 


// ----------------- SPRITE ZŁOTA (pixel-art) ---------
// Rozmiar sprite'a (8x8) - to TYLKO do definicji tablicy
// KSZTAŁT 8×8.
static bool goldSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
    {1,0,0,0,0,0,0,1},
    {0,1,0,0,0,0,1,0},
    {0,0,1,0,0,1,0,0},
    {0,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

// ----------------- SPRITE ZŁOTA podwojnego (pixel-art) ---------
// KSZTAŁT 8×8.
static bool gold2Sprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
    {1,0,0,0,0,0,0,1},
    {0,1,0,0,0,0,1,0},
    {0,0,1,0,0,1,0,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,0},
    {1,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0}
};

// ----------------- SPRITE STARTU (pixel-art) ---------
// KSZTAŁT 8×8.
static bool startSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
{
    {0,0,1,1,1,1,0,0},
    {0,0,1,0,0,1,0,0},
    {0,1,0,0,0,0,1,0},
    {0,1,0,1,0,0,1,0},
    {0,1,1,1,0,0,1,0},
    {0,1,0,1,0,0,1,0},
    {0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0}
};

// Rysujemy pixel-art GRACZA w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
//float dla wersji postaci z liniami pomiedzy pikselami


/*void drawPlayerSprite(SDL_Renderer* renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PLAYER_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)PLAYER_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5

    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = playerSprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? COLOR_PLAYER : COLOR_PATH;
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
*/


void drawPlayerSprite(SDL_Renderer* renderer, int x, int y)
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
void drawPlayerSpriteRight(SDL_Renderer* renderer, int x, int y)
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
void drawPlayerSpriteDown(SDL_Renderer* renderer, int x, int y)
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
void drawPlayerSpriteLeft(SDL_Renderer* renderer, int x, int y)
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
void drawGoldSprite(SDL_Renderer* renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)GOLD_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)GOLD_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5

    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
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
void drawGold2Sprite(SDL_Renderer* renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)GOLD2_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)GOLD2_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
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
void drawPantherSprite(SDL_Renderer* renderer, float x, float y, bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSprite[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER): COLOR_PATH;
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
void drawPantherSpriteRight(SDL_Renderer* renderer, float x, float y, bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteRight[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER): COLOR_PATH;
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
void drawPantherSpriteLeft(SDL_Renderer* renderer, float x, float y, bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteLeft[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER): COLOR_PATH;
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
void drawPantherSpriteDown(SDL_Renderer* renderer, float x, float y, bool disabled)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)PANTHER_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)PANTHER_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
        {
            bool pixelOn = pantherSpriteDown[row][col];
            // Wybieramy kolor: zapalony = zielony, zgaszony = tło
            SDL_Color c = pixelOn ? (disabled ? COLOR_PANTHER_DISABLED : COLOR_PANTHER): COLOR_PATH;
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



// Rysujemy pixel-art IKONA STARTU GRACZA w miejscu (x,y) o szerokości i wysokości
// docelowej 20×20 (czyli skala 2.5, bo sprite ma 8×8).
void drawStartSprite(SDL_Renderer* renderer, float x, float y)
{
    // Obliczamy skalę tak, by sprite 8×8 zmieścił się w 20×20
    float scaleX = (float)START_WIDTH  / (float)SPRITE_WIDTH;   // 20 / 8 = 2.5
    float scaleY = (float)START_HEIGHT / (float)SPRITE_HEIGHT;  // 20 / 8 = 2.5
    for(int row = 0; row < SPRITE_HEIGHT; row++)
    {
        for(int col = 0; col < SPRITE_WIDTH; col++)
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
bool checkCollisionWithWalls(float newX, float newY, const std::vector<std::vector<int>>& maze) 
{
    float left   = newX;
    float top    = newY;
    float right  = newX + PLAYER_WIDTH  - 1;
    float bottom = newY + PLAYER_HEIGHT - 1;

    auto isWallAtPixel = [&](float px, float py) {
        if (px < 0 || py < 0) return true; // poza mapą
        int tileX = (int)(px / CELL_SIZE);
        int tileY = (int)(py / CELL_SIZE);
        if (tileY < 0 || tileY >= (int)maze.size() ||
            tileX < 0 || tileX >= (int)maze[tileY].size())
        {
            return true; // poza tablicą
        }
        return (maze[tileY][tileX] == 1);
    };

    if (isWallAtPixel(left,  top))    return true;
    if (isWallAtPixel(right, top))    return true;
    if (isWallAtPixel(left,  bottom)) return true;
    if (isWallAtPixel(right, bottom)) return true;

    return false;
}

// Funkcja do obliczenia, dokąd można dojść w danym kierunku (dx, dy) aż do ściany.
void computeTargetCell(
    const std::vector<std::vector<int>>& maze,
    int startCellX, int startCellY,
    int dx, int dy,
    int& outX, int& outY)
{
    int newX = startCellX;
    int newY = startCellY;

    while (true) {
        int testX = newX + dx;
        int testY = newY + dy;

        if (testY < 0 || testY >= (int)maze.size() ||
            testX < 0 || testX >= (int)maze[testY].size()) {
            break; // poza tablicą
        }
        if (maze[testY][testX] == 1) {
            break; // ściana
        }
        newX = testX;
        newY = testY;
    }
    outX = newX;
    outY = newY;
}

// Funkcja do rysowania prostokąta - używana przy ścianach, złocie, itp.
void drawRect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

// Funkcja do rysowania tekstu
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create text texture: " << TTF_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// GŁÓWNA PĘTLA PROGRAMU-----------------------------------------------------
int main(int argc, char* argv[])
{

// Szablon labiryntu
    std::vector<std::vector<int>> templateMaze = {
        {5, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1}, 
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };



    // Licznik punktów, flaga złota
    int score = 0;
    bool hasGold = false;

    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    // Inicjalizacja SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    // Wczytanie czcionki
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 21);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Okno
    SDL_Window* window = SDL_CreateWindow(
        "Raid on Fort Knox - Pixel sprite",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Labirynt 1 (1=ściana, 0=ścieżka, 5=życie, 4 - złoto, 3 - złoto podwójne)
    std::vector<std::vector<int>> maze2 = {
        {5, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1}, 
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 4, 3, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };
// Labirynt 1 (1=ściana, 0=ścieżka, 5=życie, 4 - złoto, 3 - złoto podwójne)
    std::vector<std::vector<int>> maze = {
    
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
{1,3,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1},
{1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1},
{1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1},
{1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1},
{1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,1},
{1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1},
{1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1},
{1,0,1,1,1,0,1,1,0,1,1,1,1,1,1,0,1,0,1,1,1,1},
{1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1},
{1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
{1,4,1,0,0,0,1,0,0,0,0,0,0,4,1,0,0,0,1,0,1,1},
{1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1},
{1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1},
{1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
{1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1},
{1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1},
{1,0,0,0,1,0,1,0,0,0,1,3,0,0,1,0,0,0,1,0,1,1},
{1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
{1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1},
{1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
{1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,1},
{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

          };

    // Startowa pozycja (wycentrowana w kafelku 19,1)
    posX = 19.0f * CELL_SIZE + (CELL_SIZE - PLAYER_WIDTH) / 2.0f;
    posY =  1.0f * CELL_SIZE + (CELL_SIZE - PLAYER_HEIGHT)/ 2.0f;

    posXstart = 19.0f * CELL_SIZE + (CELL_SIZE - GOLD_WIDTH) / 2.0f;
    posYstart =  0.0f * CELL_SIZE + (CELL_SIZE - GOLD_HEIGHT)/ 2.0f;
    
    // Początkowo stoimy w miejscu
    targetPosX = posX;
    targetPosY = posY;

    bool running = true;
    SDL_Event event;

    while (running) {
        // 1) Obsługa zdarzeń
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } 
            else if (event.type == SDL_KEYDOWN) {
                // Określamy aktualną komórkę (środek gracza)
                int cellX = (int)((posX + PLAYER_WIDTH/2) / CELL_SIZE);
                int cellY = (int)((posY + PLAYER_HEIGHT/2) / CELL_SIZE);

                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                    // Snapujemy w osi X (wyśrodkowujemy)
                    float centerX = cellX * CELL_SIZE + CELL_SIZE / 2.0f;
                    posX = centerX - PLAYER_WIDTH / 2.0f;

                    // Kierunek
                    dirCellX = 0;
                    dirCellY = (event.key.keysym.sym == SDLK_UP) ? -1 : 1;
                }
                else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT) {
                    // Snapujemy w osi Y
                    float centerY = cellY * CELL_SIZE + CELL_SIZE / 2.0f;
                    posY = centerY - PLAYER_HEIGHT / 2.0f;

                    dirCellY = 0;
                    dirCellX = (event.key.keysym.sym == SDLK_LEFT) ? -1 : 1;
                }
                else {
                    // np. spacja, ESC, itp. - ignorujemy
                    continue;
                }

                // Teraz obliczamy docelową komórkę
                cellX = (int)((posX + PLAYER_WIDTH/2) / CELL_SIZE);
                cellY = (int)((posY + PLAYER_HEIGHT/2) / CELL_SIZE);

                int tCellX, tCellY;
                computeTargetCell(maze, cellX, cellY, dirCellX, dirCellY, tCellX, tCellY);

                // Zamieniamy na piksele (też wycentrowane)
                targetPosX = tCellX * CELL_SIZE + (CELL_SIZE - PLAYER_WIDTH)/2.0f;
                targetPosY = tCellY * CELL_SIZE + (CELL_SIZE - PLAYER_HEIGHT)/2.0f;

                isMoving = true;
            }
        }

        // 2) Logika płynnego ruchu
        if (isMoving) {
            float dx = targetPosX - posX;
            float dy = targetPosY - posY;
            float dist = std::sqrt(dx*dx + dy*dy);

            if (dist > speed) {
                float stepX = posX + speed * (dx / dist);
                float stepY = posY + speed * (dy / dist);

                // Sprawdzamy kolizję - wąski korytarz, raczej i tak jej nie będzie,
                // ale zostawiamy "na wszelki wypadek".
                if (!checkCollisionWithWalls(stepX, stepY, maze)) {
                    posX = stepX;
                    posY = stepY;
                } else {
                    // Kolizja - zatrzymujemy ruch
                    isMoving = false;
                }
            } else {
                // Ostatni krok
                float stepX = targetPosX;
                float stepY = targetPosY;

                if (!checkCollisionWithWalls(stepX, stepY, maze)) {
                    posX = stepX;
                    posY = stepY;
                }
                isMoving = false;
            }
             if (pantherIsDisabled) {
        pantherDisableTimer--;
        if (pantherDisableTimer <= 0) {
            pantherIsDisabled = false;
            std::cout << "[DEBUG] Pantera znowu niebezpieczna.\n";
        }
    }
        }

        // 3) Sprawdzamy, czy zbieramy złoto / wejście na start
        {
            int checkCellX = (int)((posX + PLAYER_WIDTH/2) / CELL_SIZE);
            int checkCellY = (int)((posY + PLAYER_HEIGHT/2) / CELL_SIZE);

            if (checkCellY >= 0 && checkCellY < (int)maze.size() &&
                checkCellX >= 0 && checkCellX < (int)maze[checkCellY].size())
            {
                int cellValue = maze[checkCellY][checkCellX];
                if (cellValue == 4) {
                    
                    if (hasGold) {
                        isMoving = false;
                    } else if (!hasGold) {
                        std::cout << "Zbierasz zloto pojedyncze\n";
                        maze[checkCellY][checkCellX] = 0;
                        hasGold = true;
                        if (hasGold) {
                         isMoving = false;
                        }
                    }
                }
                else if (cellValue == 3) {
                 if (hasGold) {
                    isMoving = false;
                 } else if (!hasGold) {
                        std::cout << "Zbierasz zloto podwojne\n";
                        maze[checkCellY][checkCellX] = 4;
                        hasGold = true;
                        if (hasGold) {
                        isMoving = false;
                    }
                }
                }
                // kolizja ze startem
                        if (checkStartBoxCollision(posX, posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        posXstart, posYstart, START_WIDTH, START_HEIGHT)) {
                            if (hasGold) {
                                    std::cout << "Oddajesz zloto na start!\n";
                                    score++;
                                    hasGold = false;}
                                 else {
                                    std::cout << "Nie masz złota!\n";
                                    }
                        }
                else if (cellValue == 5) {
                    // Tu np. mechanika życia
                }
            
if (!pantherIsDisabled) {
                // Kolizja z panterą
                        if (checkPantherBoxCollision(posX, posY, PLAYER_WIDTH, PLAYER_HEIGHT,
                        pantherX, pantherY, PANTHER_WIDTH, PANTHER_HEIGHT)) 
                      {
                            if (!justCollidedWithPanther) {
                                if (hasGold) {
                                    justCollidedWithPanther = true;
                                    pantherIsDisabled = true;
                                    pantherDisableTimer = PANTHER_DISABLE_TIME;
                                    std::cout << "Złapała Cię pantera. Tracisz złoto!\n";
                                    hasGold = false;
                                } else {
                                    std::cout << "Złapała Cię pantera. Tracisz życie!\n";
                                    // Usuwasz jedno życie z labiryntu lub zmniejszasz licznik "lives"
                                    maze[0][2] = 0;
                                    }
                            } else {
    // tu "else" jest wobec warunku cellValue == 7
    // tzn. jeżeli to *nie* pantera, to znaczy, że gracz stoi gdzieś indziej
    // i można zresetować justCollidedWithPanther
                                justCollidedWithPanther = false;
                                }
                        }
                                /////////////////////////// ruszanie pantery
                                // Przesunięcie pantery w bieżącym kierunku
                                pantherX += pantherSpeed * pantherDirX; // Ruch w poziomie
                                pantherY += pantherSpeed * pantherDirY; // Ruch w pionie

                                // Sprawdzanie kolizji z korytarzem (ścianami)
                                int cellX = (int)(pantherX / CELL_SIZE); // Pozycja w siatce
                                int cellY = (int)(pantherY / CELL_SIZE);

                                // Jeśli pantera napotka ścianę, zmień kierunek
                                if (maze[cellY][cellX] == 1) { // 1 = ściana
                                if (pantherDirX != 0) { // Porusza się w poziomie
                                pantherDirX *= -1; // Zmiana kierunku na osi X
                                } else if (pantherDirY != 0) { // Porusza się w pionie
                                pantherDirY *= -1; // Zmiana kierunku na osi Y
                                }
                            }      
                                ////////////////////////////////////////// koniec ruszanie pantery
                    }
            }
    }        
        // 4) Renderowanie
        SDL_SetRenderDrawColor(renderer, COLOR_PATH.r, COLOR_PATH.g, COLOR_PATH.b, COLOR_PATH.a);
        SDL_RenderClear(renderer);



        // Rysowanie labiryntu
        for (size_t y = 0; y < maze.size(); ++y) {
            for (size_t x = 0; x < maze[y].size(); ++x) {
                int val = maze[y][x];
                if (val == 1) {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_WALL);
                } else if (val == 4) {
                    drawGoldSprite(renderer, x * CELL_SIZE, y * CELL_SIZE);
                } else if (val == 3) {
                    drawGold2Sprite(renderer, x * CELL_SIZE, y * CELL_SIZE);
                } else if (val == 5) {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_LIVES);
                } //else if (val == 6) {
                    //drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_START);
                //}  
                else {
                    drawRect(renderer, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_PATH);
                }
            }
        }

        // Rysowanie punktów
        std::string scoreText = "Punkty: " + std::to_string(score);
        renderText(renderer, font, scoreText, 5 * CELL_SIZE, 0, {255, 255, 255, 255});

        // *** RYSOWANIE GRACZA: pixel-art sprite ***

        // sprawdzamy, czy gracz idzie w lewo, prawo, górę, dół
        if (!isMoving) {
            drawPlayerSprite(renderer, posX, posY);
        }
          else if (dirCellX == 1) {
            drawPlayerSpriteRight(renderer, posX, posY);
        } else if (dirCellX == -1) {
            drawPlayerSpriteLeft(renderer, posX, posY);
        } else if (dirCellY == 1) {
            drawPlayerSpriteDown(renderer, posX, posY);
        } else if (dirCellY == -1) {
            drawPlayerSprite(renderer, posX, posY);
        //} else {
        //drawPlayerSprite(renderer, posX, posY);
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
