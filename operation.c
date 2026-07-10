#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <string.h>
#include "format.h"
#include "operation.h"
_Bool check_pos(Detail *detail, int posX, int posY) {
    if (!(posX >= 0 && posX <= detail->width && posY >= 0 && posY <= detail->height)) {
        return 0;
    } else {
        return 1;
    }
}
void point(FILE *fp, Detail *detail, unsigned char *color, int posX, int posY) {
    if (check_pos(detail, posX, posY)) {
        if (detail->type == bit_1) {
            Pixel_1 *data1 = (Pixel_1*)detail->data;    // read
            int bit_pos = 7 - (posX % 8);
            data1[((detail->width + detail->padding) * (posY - 1))/8  + posX/8].bits ^= (1 << bit_pos);
            detail->data = data1;                       // write
        } else if (detail->type == bit_24) {
            Pixel_24 *data2 = (Pixel_24*)detail->data;  // read
            data2[((detail->width + detail->padding) * (posY - 1)) + posX].b = color[2];
            data2[((detail->width + detail->padding) * (posY - 1)) + posX].g = color[1];
            data2[((detail->width + detail->padding) * (posY - 1)) + posX].r = color[0];
            detail->data = data2;                       // write
        }
    } else {
        return;
    }
}

void line(FILE *fp, Detail *detail, unsigned char *color, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y) {

    int sideY = abs(pos2_Y - pos1_Y);
    int sideX = abs(pos2_X - pos1_X);
    int num = 0;
    char short_side = 0;
    int step_x, step_y;

    if (sideY > sideX) {
        num = sideX;
        short_side = 'x';
        if (pos2_Y > pos1_Y) {
            step_y = sideY/(num - 1);
        } else if (pos2_Y < pos1_Y) {
            step_y = -sideY/(num - 1);
        } else {
            step_y = 0;
        }

        if (pos2_X > pos1_X) {
            step_x = 1;
        } else if (pos2_X < pos2_X){
            step_x = -1;
        } else {
            step_x = 0;
        }
    } else if (sideY <= sideX){
        num = sideY;
        short_side = 'y';
        if (pos2_X > pos1_X) {
            step_x = sideX/(num - 1);
        } else if (pos2_X <= pos1_X){
            step_x = -sideX/(num - 1);
        } else {
            step_x = 0;
        }

        if (pos2_Y > pos1_Y) {
            step_y = 1;
        } else if (pos2_Y < pos2_Y){
            step_y = -1;
        } else {
            step_y = 0;
        }
        
    }
    Pixel_Pos *array = malloc((num + 1)*sizeof(Pixel_Pos));

    array[0].posX = pos1_X;
    array[0].posY = pos1_Y;
    for (int i = 1; i < num; i++) {
        array[i].posX = array[i-1].posX + step_x;
        array[i].posY = array[i-1].posY + step_y;
    }
    array[num].posX = pos2_X;
    array[num].posY = pos2_Y;
    
    for (int i = 0; i <= num; i++) {
        point(fp, detail, color, array[i].posX, array[i].posY);
    }
    free(array);
    return;
} 

void rect(FILE *fp, Detail *detail, unsigned char *color, int pos1_X, int pos1_Y, int pos2_X, int pos2_Y) {

    Pixel_Pos pos3;
    Pixel_Pos pos4;
    pos3.posX = pos1_X;
    pos3.posY = pos2_Y;

    pos4.posX = pos2_X;
    pos4.posY = pos1_Y;

    line(fp, detail, color, pos1_X, pos1_Y, pos3.posX, pos3.posY);
    line(fp, detail, color, pos1_X, pos1_Y, pos4.posX, pos4.posY);
    line(fp, detail, color, pos2_X, pos2_Y, pos3.posX, pos3.posY);
    line(fp, detail, color, pos2_X, pos2_Y, pos3.posX, pos3.posY);
}

void circle(FILE *fp, Detail *detail, unsigned char *color, int radius, int pos_X, int pos_Y) {
    for (int i = pos_X - radius; i < pos_X + radius; i++) {
        for (int j = pos_Y - radius; j < pos_Y + radius; j++) {
            if (sqrt((i-pos_X)*(i-pos_X) + (j-pos_Y)*(j-pos_Y)) >= radius-1 || 
            sqrt((i-pos_X)*(i-pos_X) + (j-pos_Y)*(j-pos_Y)) <= radius+1) {
                point(fp, detail, color, pos_X, pos_Y);
            }
        }
    }
    return;
}

int get_bmp_data(FILE *fp, Detail *src_detail) {
    unsigned char buffer[4] = {0};
    fseek(fp, 10*sizeof(uint8_t), SEEK_SET);
    memset(buffer, 0, 4);
    fread(buffer, 1, 4, fp);
    buffer_reverse(buffer);
    sscanf(buffer, "%d", src_detail->image_offset);
    
    fseek(fp, 18*sizeof(uint8_t), SEEK_SET);
    memset(buffer, 0, 4);
    fread(buffer, 1, 4, fp);
    buffer_reverse(buffer);
    sscanf(buffer, "%d", src_detail->width);
    
    fseek(fp, 22*sizeof(uint8_t), SEEK_SET);
    memset(buffer, 0, 4);
    fread(buffer, 1, 4, fp);
    buffer_reverse(buffer);
    sscanf(buffer, "%d", src_detail->height);
    
    fseek(fp, 28*sizeof(uint8_t), SEEK_SET);
    memset(buffer, 0, 4);
    fread(buffer, 1, 2, fp);
    buffer_reverse(buffer);
    sscanf(buffer, "%d", src_detail->one_pixel_bit_size);
    
    if (src_detail->one_pixel_bit_size == 1) {
        src_detail->type = bit_1;
    } else if (src_detail->one_pixel_bit_size == 24) {
        src_detail->type = bit_24;
    } else {
        return ERROR;
    }
    
    fseek(fp, 34*sizeof(uint8_t), SEEK_SET);
    memset(buffer, 0, 4);
    fread(buffer, 1, 4, fp);
    buffer_reverse(buffer);
    sscanf(buffer, "%d", src_detail->image_size);

    src_detail->padding = (src_detail->image_size - src_detail->height*src_detail->width)/src_detail->height;
    

}
void paste(FILE *fp_targ, Detail *detail, Detail *src_detail, int pos_X, int pos_Y, char *path) {
    FILE *fp_src = fopen(path, "r");
    if (fp_src == NULL) return;
    get_bmp_data(fp_src, src_detail);

   
    detail->data = update_image_data(fp_targ, detail, src_detail, fp_src);

    write_image_data(fp_targ, detail);
}


void show(Detail *detail) {
    printf("width : %d\n", detail->width);
    printf("height : %d\n", detail->height);
    printf("padding : %d\n", detail->padding);
    printf("type : %d(0 : bit_1, 1 : bit_24)\n", detail->type);
    printf("one_pixel_bit_size : %d\n", detail->one_pixel_bit_size);
    printf("image_size : %d\n", detail->image_size);
    printf("image_offset : %d\n", detail->image_offset);
    printf("bg_color : %s\n", detail->bg_color);
}
void help() {
    printf("========================================\n");
    printf("=\tpaint command\n");
    printf("================basic===================\n");
    printf("exit\n");
    printf("help\n");
    printf("show\n");
    printf("================main====================\n");
    printf("clear\n");
    printf("point + <color> <pixelX> <pixelY>\n");
    printf("line + <color> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("rect + <color> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("circle + <color> <pixelX> <pixelY> <radius>\n");
    printf("paste + <path> <pixelX> <pixelY> \n");
    printf("========================================\n");

}

void clear(Detail *detail, FILE *fp) {
    detail->data = update_image_data(fp, detail, NULL, NULL);
    write_image_data(fp, detail);
}