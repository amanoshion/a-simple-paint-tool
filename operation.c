#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <string.h>
#include "format.h"
#include "operation.h"

_Bool check_pos(Detail *detail, int posX, int posY) {
    if (posX >= 0 && posX < detail->width && posY >= 0 && posY < detail->height) {
        return OK;
    } else {
        printf("pos out of screen x : %d y : %d\n", posX, posY);
        return 0;
    }
}

void draw_one_pixel(FILE *fp, Detail *detail, uint8_t *color, int posX, int posY) {
    if (detail == NULL || fp == NULL) return;
    if (check_pos(detail, posX, posY) == 0) return;
    if (color == NULL || detail->data == NULL) return;
    if (detail->type == bit_1) {
        uint8_t *data = (uint8_t*)detail->data;
        uint32_t bit_offset = posY * (detail->width + detail->padding_in_bits) + posX;
        uint32_t byte_off = bit_offset / 8;
        uint32_t bit_in_byte = bit_offset % 8;
        uint8_t bit_value = (color[0] || color[1] || color[2]) ? 1 : 0;
        if (bit_value) {
            data[byte_off] |= (0x80 >> bit_in_byte);
        } else {
            data[byte_off] &= ~(0x80 >> bit_in_byte);
        }
        
    } else if (detail->type == bit_24) {
        uint8_t *data = (uint8_t *)detail->data;
        uint32_t bytes_per_row = detail->width * 3 + detail->padding_in_bytes;
        uint32_t byte_off = posY * bytes_per_row + posX * 3;

        data[byte_off] = color[2];      // B        // TODO : FIX
        data[byte_off+1] = color[1];    // G
        data[byte_off+2] = color[0];    // R

    } else if (detail->type == undefined) {
        printf("type undefined\n");
    }
    return;
}

void point(FILE *fp, Detail *detail, uint8_t *color, int size, int posX, int posY) {
    if (size < 1) return;

    int x1 = posX - (size - 1);
    int x2 = posX + (size - 1);
    int y1 = posY - (size - 1);
    int y2 = posY + (size - 1);
    int dx, dy;
    for(int i = x1; i <= x2; i++) {
        for(int j = y1; j <= y2; j++) {
            dx = abs(i - posX);
            dy = abs(j - posY);
            if (sqrt(dx*dx + dy*dy) <= size) {
                draw_one_pixel(fp, detail, color, i, j);
            }
        }
    }
    write_image_data(fp, detail);
    return;
}

void line(FILE *fp, Detail *detail, uint8_t *color, int size, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y) {
    int sideY = abs(pos2_Y - pos1_Y);
    int sideX = abs(pos2_X - pos1_X);
    int num = 0;
    int step_x, step_y;

    if (sideY > sideX) {
        num = sideX;        
        if (num <= 1) num = sideY;
        if (pos2_Y > pos1_Y) {
            step_y = ceil(sideY / num); // avoid step_x/step_y be 0
        } else if (pos2_Y < pos1_Y) {
            step_y = floor(-sideY / num);
        } else {
            step_y = 0;
        }
        
        if (pos2_X > pos1_X) {
            step_x = 1;
        } else if (pos2_X < pos1_X){
            step_x = -1;
        } else {
            step_x = 0;
        }
    } else if (sideY <= sideX){
        num = sideY;
        if (num <= 1) num = sideX;
        if (pos2_X > pos1_X) {
            step_x = ceil(sideX / num);       
        } else if (pos2_X < pos1_X){
            step_x = floor(-sideX / num); 
        } else {
            step_x = 0;
        }

        if (pos2_Y > pos1_Y) {
            step_y = 1;
        } else if (pos2_Y < pos1_Y){
            step_y = -1;
        } else {
            step_y = 0;
        }
        
    }

    Pixel_Pos *array = malloc(num * sizeof(Pixel_Pos));

    for (int i = 0; i < num; i++) {
        array[0].posX = pos1_X;
        array[0].posY = pos1_Y;
        for (int i = 1; i < num; i++) {
            array[i].posX = array[i-1].posX + step_x;
            array[i].posY = array[i-1].posY + step_y;
        }
        // array[num - 1].posX = pos2_X;
        // array[num - 1].posY = pos2_Y;
        
        point(fp, detail, color, size, array[i].posX, array[i].posY);
        // printf("i: %d, x: %d y: %d\n", i, array[i].posX, array[i].posY);
    }
    free(array);
    return;
} 

void rect(FILE *fp, Detail *detail, uint8_t *color, int size, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y) {

    Pixel_Pos pos3;
    Pixel_Pos pos4;
    pos3.posX = pos1_X;
    pos3.posY = pos2_Y;

    pos4.posX = pos2_X;
    pos4.posY = pos1_Y;

    line(fp, detail, color, size, pos1_X, pos1_Y, pos3.posX, pos3.posY);
    line(fp, detail, color, size, pos1_X, pos1_Y, pos4.posX, pos4.posY);
    line(fp, detail, color, size, pos2_X, pos2_Y, pos3.posX, pos3.posY);
    line(fp, detail, color, size, pos2_X, pos2_Y, pos4.posX, pos4.posY);

    return;
}

void circle(FILE *fp, Detail *detail, uint8_t *color, int size, int pos_X, int pos_Y,  int radius) {
    for (int i = pos_X - radius; i <= pos_X + radius; i++) {
        for (int j = pos_Y - radius; j < pos_Y + radius; j++) {
            if (sqrt((i-pos_X)*(i-pos_X) + (j-pos_Y)*(j-pos_Y)) >= radius-1 && 
            sqrt((i-pos_X)*(i-pos_X) + (j-pos_Y)*(j-pos_Y)) <= radius) {
                point(fp, detail, color, size, i, j);
            }
        }
    }
    return;
}


void paste(FILE *fp, Detail *detail, FILE *fp_src, Detail *src_detail, int pos_X, int pos_Y) {
    if (fp_src == NULL) return;
    get_bmp_data(fp_src, src_detail);

    update_image_data(fp, detail, fp_src, src_detail, pos_X, pos_Y);

    write_image_data(fp, detail);
}


void show(Detail *detail) {
    printf("width : %d\n", detail->width);
    printf("height : %d\n", detail->height);
    if (detail->type == bit_1) {
        printf("padding : %d bits\n", detail->padding_in_bits);
    } else if (detail->type == bit_24) {
        printf("padding : %d bytes\n", detail->padding_in_bytes);
    } else {
        printf("error : lost padding\n");
    }
    printf("type : %d(0 : bit_1, 1 : bit_24)\n", detail->type);
    printf("one_pixel_bit_size : %d\n", detail->one_pixel_bit_size);
    printf("image_size : %d\n", detail->image_size);
    printf("image_offset : %d\n", detail->image_offset);
    printf("bg_color(rgb) : %X%X%X\n", detail->bg_color[2], detail->bg_color[1], detail->bg_color[0]);
    printf("========================================\n");
}
void help() {
    printf("========================================\n");
    printf("\t[paint tool command]\n");
    printf("================basic===================\n");
    printf("exit\n");
    printf("help\n");
    printf("show\n");
    printf("================main====================\n");
    printf("clear\n");
    printf("point + <color> <size> <pixelX> <pixelY>\n");
    printf("line + <color> <size> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("rect + <color> <size> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("circle + <color> <size> <pixelX> <pixelY> <radius>\n");
    printf("paste\n");
    printf("========================================\n");
}

void clear(Detail *detail, FILE *fp) {
    update_image_data(fp, detail, NULL, NULL, 0, 0);
    write_image_data(fp, detail);
}