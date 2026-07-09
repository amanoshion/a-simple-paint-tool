#include <stdio.h>
#include "format.h"
void *point(FILE *fp, Detail *detail, unsigned char *color, int posX, int posY) {
    if (!(posX > 0 && posX < detail->width && posY > 0 && posY < detail->height)) return ERROR;
    if (detail->type == bit_1) {
        Pixel_1 *data1 = (Pixel_1*)detail->data;
        int bit_pos = 7 - (posX % 8);
        data1[((detail->width + detail->padding) * (posY - 1))/8  + posX/8].bits ^= (1 << bit_pos);
        return data1;
    } else if (detail->type == bit_24) {
        Pixel_24 *data2 = (Pixel_24*)detail->data;
        data2[((detail->width + detail->padding) * (posY - 1)) + posX].b = color[2];
        data2[((detail->width + detail->padding) * (posY - 1)) + posX].g = color[1];
        data2[((detail->width + detail->padding) * (posY - 1)) + posX].r = color[0];
        return data2;
    }
}
