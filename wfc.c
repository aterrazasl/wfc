#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "wfc.h"

static char memBuffer[BOARD_HEIGHT * TILE_HEIGHT][BOARD_WIDTH * TILE_WIDTH];

static tile_t board[BOARD_HEIGHT][BOARD_WIDTH];

static rules_t rules[5];

static void createRules(rules_t *rules)
{
    //This rules describes the allowed indices for each side of the tile
    rules[0] = (rules_t){.up    = (dir_t){.a = {0, 1, 0}, .size = 2},
                         .right = (dir_t){.a = {0, 2, 0}, .size = 2},
                         .down  = (dir_t){.a = {0, 3, 0}, .size = 2},
                         .left  = (dir_t){.a = {0, 4, 0}, .size = 2}};
    rules[1] = (rules_t){.up    = (dir_t){.a = {2, 3, 4}, .size = 3},
                         .right = (dir_t){.a = {3, 4, 1}, .size = 3},
                         .down  = (dir_t){.a = {0, 3, 0}, .size = 2},
                         .left  = (dir_t){.a = {2, 3, 1}, .size = 3}};
    rules[2] = (rules_t){.up    = (dir_t){.a = {3, 4, 2}, .size = 3},
                         .right = (dir_t){.a = {1, 3, 4}, .size = 3},
                         .down  = (dir_t){.a = {1, 4, 2}, .size = 3},
                         .left  = (dir_t){.a = {0, 4, 0}, .size = 2}};
    rules[3] = (rules_t){.up    = (dir_t){.a = {0, 1, 0}, .size = 2},
                         .right = (dir_t){.a = {1, 4, 3}, .size = 3},
                         .down  = (dir_t){.a = {1, 2, 4}, .size = 3},
                         .left  = (dir_t){.a = {1, 2, 3}, .size = 3}};
    rules[4] = (rules_t){.up    = (dir_t){.a = {2, 3, 4}, .size = 3},
                         .right = (dir_t){.a = {0, 2, 0}, .size = 2},
                         .down  = (dir_t){.a = {1, 2, 4}, .size = 3},
                         .left  = (dir_t){.a = {1, 2, 3}, .size = 3}};
}

static void insertItem(list_t *list, tile_t *item)
{
    if (list->len < MAX_NEIGHBORS)
    {
        list->list[list->len] = (void *)item;
    }
    list->len++;
}

static int genRand(int min, int max)
{
    return ((double)rand() / RAND_MAX) * (max - (-min)) + (-min);
}

static void insertBoardTile(int i, int j, const char *t,char charBoardp[BOARD_HEIGHT * TILE_HEIGHT][BOARD_WIDTH * TILE_WIDTH])
{
    assert(t != NULL && "tile cannot be NULL");

    int yoffset = j * TILE_WIDTH;
    int xoffset = i * TILE_HEIGHT;

    for (int y = 0; y < TILE_HEIGHT; y++)
    {
        for (int x = 0; x < TILE_WIDTH; x++)
        {
            charBoardp[xoffset + x][yoffset + y] = t[(y + (x * TILE_WIDTH))];
        }
    }
}

static void insertBoardInBuffer(tile_t boardp[BOARD_HEIGHT][BOARD_WIDTH],char charBoardp[BOARD_HEIGHT * TILE_HEIGHT][BOARD_WIDTH * TILE_WIDTH])
{
    for (int x = 0; x < BOARD_HEIGHT; x++)
    {
        for (int y = 0; y < BOARD_WIDTH; y++)
        {
            insertBoardTile(x, y, boardp[x][y].tile,charBoardp);
        }
    }
}

static void findNeighbors(tile_t boartp[BOARD_HEIGHT][BOARD_WIDTH])
{
    for (int x = 0; x < BOARD_HEIGHT; x++)
    {
        for (int y = 0; y < BOARD_WIDTH; y++)
        {
            for (int n = 0; n < MAX_NEIGHBORS; n++)
            {
                switch (n)
                {
                case 0: // up
                    if (x != 0)
                        insertItem(&boartp[x][y].neighbors, &boartp[x - 1][y]);
                    else
                        insertItem(&boartp[x][y].neighbors, NULL);
                    break;
                case 1: // right
                    if (y != BOARD_WIDTH - 1)
                        insertItem(&boartp[x][y].neighbors, &boartp[x][y + 1]);
                    else
                        insertItem(&boartp[x][y].neighbors, NULL);
                    break;
                case 2: // down
                    if (x != BOARD_HEIGHT - 1)
                        insertItem(&boartp[x][y].neighbors, &boartp[x + 1][y]);
                    else
                        insertItem(&boartp[x][y].neighbors, NULL);
                    break;
                case 3: // left
                    if (y != 0)
                        insertItem(&boartp[x][y].neighbors, &boartp[x][y - 1]);
                    else
                        insertItem(&boartp[x][y].neighbors, NULL);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static void initBoard(tile_t boartp[BOARD_HEIGHT][BOARD_WIDTH])
{
    for (int x = 0; x < BOARD_HEIGHT; x++)
    {
        for (int y = 0; y < BOARD_WIDTH; y++)
        {
            boartp[x][y].entropy = TILE_COUNT;
            boartp[x][y].collapsed = NOT_COLLAPSED;
            boartp[x][y].tile = (char *)&emptyTile[0];
            boartp[x][y].pos.row = x;
            boartp[x][y].pos.col = y;
        }
    }
}

static void printBuffer(char charBoardp[BOARD_HEIGHT * TILE_HEIGHT][BOARD_WIDTH * TILE_WIDTH])
{
    for (int j = 0; j < BOARD_HEIGHT * TILE_HEIGHT; j++)
    {
        for (int i = 0; i < BOARD_WIDTH * TILE_WIDTH; i++)
        {
            printf("%c", charBoardp[j][i]);
        }
        printf("\r\n");
    }
    printf("\r\n");
}

static tile_t *findLowestEntropyTile(tile_t boartp[BOARD_HEIGHT][BOARD_WIDTH])
{
    tile_t *t = &boartp[0][0];

    for (int x = 0; x < BOARD_HEIGHT; x++)
    {
        for (int y = 0; y < BOARD_WIDTH; y++)
        {
            if ((boartp[x][y].entropy < t->entropy) && (boartp[x][y].collapsed == NOT_COLLAPSED))
            {
                t = &boartp[x][y];
            }
        }
    }
    return t;
}

static void addToArray(int *dst, int *src, int *dstCount, int srcCount)
{

    int index = 0;
    for (int i = 0; i < srcCount; i++)
    {
        *(dst + (*dstCount)) = *(src + i);
        *dstCount = *dstCount + 1;
    }
}

static int findAvailableTiles(tile_t *neighbor, dir_t *list, rules_t *rulesp)
{
    int collapsedCount = 0;

    for (int n = 0; n < neighbor->neighbors.len; n++)
    {
        if (neighbor->neighbors.list[n])
        {
            if (((tile_t *)neighbor->neighbors.list[n])->collapsed == COLLAPSED)
            {
                int tileIndex = ((tile_t *)neighbor->neighbors.list[n])->tileIndex;

                switch (n)
                {
                case 2:
                    addToArray(list[n].a, &rulesp[tileIndex].up.a[0], &list[n].size, rulesp[tileIndex].up.size);
                    collapsedCount++;
                    break;
                case 3:
                    addToArray(list[n].a, &rulesp[tileIndex].right.a[0], &list[n].size, rulesp[tileIndex].right.size);
                    collapsedCount++;
                    break;
                case 0:
                    addToArray(list[n].a, &rulesp[tileIndex].down.a[0], &list[n].size, rulesp[tileIndex].down.size);
                    collapsedCount++;
                    break;
                case 1:
                    addToArray(list[n].a, &rulesp[tileIndex].left.a[0], &list[n].size, rulesp[tileIndex].left.size);
                    collapsedCount++;
                    break;
                default:
                    break;
                }
            }
        }
    }
    return collapsedCount;
}

int findRepeated(dir_t *a, dir_t *b, dir_t *c)
{

    int repIndex = 0;
    c->size = repIndex;
    for (int i = 0; i < a->size; i++)
    {
        for (int j = 0; j < b->size; j++)
        {
            if (a->a[i] == b->a[j])
            {
                c->a[repIndex] = a->a[i];
                repIndex++;
                c->size = repIndex;
            }
        }
    }
    return repIndex;
}

int calculateEntropy(dir_t *list, int *repeated)
{

    int repCount = 0;
    dir_t *listPtr[MAX_NEIGHBORS] = {};

    int index = 0;
    for (int i = 0; i < MAX_NEIGHBORS; i++)
    {
        if (list[i].size != 0)
        {
            listPtr[index] = &list[i];
            index++;
        }
    }

    dir_t c;
    if (index > 1)
    {

        for (int i = 0; i < index - 1; i++)
        {
            findRepeated(listPtr[i], listPtr[i + 1], &c);
            repCount = c.size;
        }

        for (int i = 0; i < c.size; i++)
        {
            repeated[i] = c.a[i];
        }
    }
    else
    {
        for (int i = 0; i < listPtr[0]->size; i++)
        {
            repeated[i] = listPtr[0]->a[i];
        }

        repCount = listPtr[0]->size;
    }

    return repCount;
}

static void calculateNeighborsEntropy(rules_t * rulesp, tile_t *boartp)
{

    for (int n = 0; n < boartp->neighbors.len; n++)
    {
        if (boartp->neighbors.list[n])
        {
            if (((tile_t *)boartp->neighbors.list[n])->collapsed == NOT_COLLAPSED)
            {
                dir_t list[MAX_NEIGHBORS] = {};
                dir_t *lstPtr = list;
                int entropy = 0;
                
                // get the available tiles for this neighbor
                findAvailableTiles((tile_t *)boartp->neighbors.list[n], lstPtr, rulesp);
                entropy = calculateEntropy(lstPtr, ((tile_t *)boartp->neighbors.list[n])->available.a);
                ((tile_t *)boartp->neighbors.list[n])->available.size = entropy;
                ((tile_t *)boartp->neighbors.list[n])->entropy = entropy;
                assert(entropy != 0 && "No options found!!!");
            }
        }
    }
}

static void selectRandomAvailableTile(tile_t *t)
{
    // on first time run, all tiles have same entropy
    if (t->entropy == TILE_COUNT)
    {
        int x = genRand(0, TILE_COUNT - 1);
        t->tile = &tiles[x][0];
        t->tileIndex = x;
    }
    else
    {
        int rnd = genRand(0, t->available.size);
        t->tileIndex = t->available.a[rnd];
        t->tile = &tiles[t->tileIndex][0];
    }

    t->collapsed = COLLAPSED;
}

int main(void)
{
    srand(time(NULL));

    initBoard(board);
    findNeighbors(board);

    createRules(rules);

    int collapsedCounter = 0;
    do
    {
        // pick tile with least entropy
        tile_t * t = findLowestEntropyTile(board);

        // Select a random tile from the available tiles based on entropy
        // of the collapsed tiles
        selectRandomAvailableTile(t);

        // Calculate and update the entropy of neighbors
        // and finds the available tiles based on the collapsed tiles
        calculateNeighborsEntropy(rules, t);

        collapsedCounter++;
        
        // if all tiles are collapsed stop otherwise keep looping
    } while (collapsedCounter < BOARD_WIDTH * BOARD_HEIGHT);

    insertBoardInBuffer(board,memBuffer);
    printBuffer(memBuffer);

    return 0;
}