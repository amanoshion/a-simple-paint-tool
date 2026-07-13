#ifndef OPERATION_H
#define OPERATION_H
#include <stdio.h>
#include "format.h"

typedef struct Pixel_Pos {
    int posX;
    int posY;
    char color[3];
} Pixel_Pos;

_Bool check_pos(Detail *detail, int posX, int posY);
void draw_one_pixel(FILE *fp, Detail *detail, uint8_t *color, int posX, int posY);
void point(FILE *fp, Detail *detail, uint8_t *color, int size, int posX, int posY);
void line(FILE *fp, Detail *detail, uint8_t *color, int size, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y);
void rect(FILE *fp, Detail *detail, uint8_t *color, int size, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y);
void circle(FILE *fp, Detail *detail, uint8_t *color, int size, int pos_X, int pos_Y, int radius);
int get_bmp_data(FILE *fp, Detail *src_detail);
void paste(FILE *fp_targ, Detail *detail, FILE *fp_src, Detail *src_detail, int pos_X, int pos_Y);

void show(Detail *detail);
void help();
void clear(Detail *detail, FILE *fp);

#endif