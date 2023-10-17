#ifndef __WFC_H__
#define __WFC_H__


#define TILE_WIDTH 3
#define TILE_HEIGHT 3
#define TILE_SIZE TILE_WIDTH *TILE_HEIGHT
#define TILE_COUNT 5
#define BOARD_WIDTH 16
#define BOARD_HEIGHT 8
#define MAX_NEIGHBORS 4
#define COLLAPSED 1
#define NOT_COLLAPSED 0

typedef struct{
    int a[3];
    int size;
}dir_t;

typedef struct{
    dir_t up;
    dir_t right;
    dir_t down;
    dir_t left;
}rules_t;

typedef struct{
    int row;
    int col;
}pos_t;

typedef struct{
    void *list[MAX_NEIGHBORS];
    int len;
    int max;
}list_t;

typedef struct{
    char const *tile;
    int tileIndex;
    int entropy;
    int collapsed;
    pos_t pos;
    dir_t available;
    list_t neighbors;
}tile_t;


static const char tiles[TILE_COUNT][TILE_SIZE] = {{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                                                  {' ', '1', ' ', '~', '~', '~', ' ', ' ', ' '},
                                                  {' ', '1', ' ', ' ', '1', '~', ' ', '1', ' '},
                                                  {' ', ' ', ' ', '~', '~', '~', ' ', '1', ' '},
                                                  {' ', '1', ' ', '~', '1', ' ', ' ', '1', ' '}};
static const char emptyTile[TILE_SIZE] = {'*', '*', '*', '*', '*', '*', '*', '*', '*'};


#endif
