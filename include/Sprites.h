// ----------------- SPRITE GRACZA (pixel-art) ---------
// Rozmiar sprite'a (8x8) - to TYLKO do definicji tablicy
static const int SPRITE_WIDTH = 8;
static const int SPRITE_HEIGHT = 8;
// kształt 8×8.
// 1 = zapalony piksel, 0 = zgaszony (będzie kolorem tła).

static bool playerSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
    {
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 1, 0, 1, 1, 0, 1, 0},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {1, 0, 1, 1, 1, 1, 0, 1},
        {0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0}};

static bool playerSpriteRight[SPRITE_HEIGHT][SPRITE_WIDTH] =
    {
        {1, 0, 1, 0, 0, 0, 0, 0},
        {1, 1, 0, 1, 0, 0, 1, 0},
        {0, 1, 1, 1, 0, 1, 0, 1},
        {0, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0, 0, 1, 0},
        {0, 1, 1, 0, 1, 0, 0, 0}};

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
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 1, 0, 1, 1, 0, 1, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0} // tylne łapy
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
        {0, 0, 0, 0, 1, 0, 0, 0}};

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
        {0, 0, 0, 1, 0, 0, 0, 0}};

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

// ----------------- SPRITE ZŁOTA (pixel-art) ---------
// Rozmiar sprite'a (8x8) - to TYLKO do definicji tablicy
// KSZTAŁT 8×8.
static bool goldSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
    {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}};

// ----------------- SPRITE ZŁOTA podwojnego (pixel-art) ---------
// KSZTAŁT 8×8.
static bool gold2Sprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
    {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0}};

// ----------------- SPRITE STARTU (pixel-art) ---------
// KSZTAŁT 8×8.
static bool startSprite[SPRITE_HEIGHT][SPRITE_WIDTH] =
    {
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 1, 0, 0, 1, 0},
        {0, 1, 1, 1, 0, 0, 1, 0},
        {0, 1, 0, 1, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 0, 1, 0}};
