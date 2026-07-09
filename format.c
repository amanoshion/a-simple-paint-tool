#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "format.h"

int ini_image_data(FILE *fp, Detail *detail) {
    int padding = 0;
    int p_size = 0;
    switch(detail->type) {
        case bit_1:
            p_size = detail->one_piexel_bit_size;
            break;
        case bit_24:
            p_size = detail->one_piexel_bit_size;
            break;
    }
    int ret;
    if ((ret = ((p_size*detail->width) % 32)) != 0) {
        padding = 32 - ret;
    }
    return padding;
}

void *create_image_data(FILE *fp, Detail *detail) {
    switch(detail->type) {
        case bit_1:
            Pixel_1 *data_1 = malloc(detail->pixel_num*sizeof(Pixel_1));
            unsigned char pixel_buffer = 0;
            unsigned char color_bit_1 = (detail->color[3]&1);

            int i = 0;
            int j = 0;
            int k = 0;
            for (int j = 0; j < detail->height; j++) {
                for(int i = 0; i < detail->width + detail->padding; i++) {
                    k++;
                    pixel_buffer <<= 1;
                    if (i < detail->width) {
                        pixel_buffer |= color_bit_1; 
                    } else {
                        pixel_buffer |= 0;
                    }
                    if (k >= 7) {
                        data_1[i].bits = pixel_buffer;
                        j = 0;
                    }
                }
            } 
            return data_1;
            break;
        case bit_24:
            Pixel_24 *data_24 = malloc(detail->pixel_num*sizeof(Pixel_24));
            int i = 0;
            int j = 0;
            int k = 0;
            
            for (int j = 0; j < detail->height; j++) {
                for(int i = 0; i < detail->width; i++) {
                    data_24[i].b = detail->color[0];
                    data_24[i].g = detail->color[1];
                    data_24[i].r = detail->color[2];
                }
                for (int k = 0; k < detail->padding; k++) {
                    data_24[i].b = 0;
                    data_24[i].g = 0;
                    data_24[i].r = 0;
                }
            }
            return data_24;
            break;   
    }
}


void buffer_reverse(unsigned char *buffer) {
    int len = strlen(buffer);
    unsigned char *tmp = malloc(len*sizeof(char));
    for (int i = 0; i < len; i++) {
        tmp[len-1-i] = buffer[i];
    }
    strncpy(buffer, tmp, len);
    return;
}

int create_and_write_file_data(FILE *fp, Detail *detail) {
    bmp_file_header_t header;
    bmp_file_info_t info;
    unsigned char buffer_16[2] = {0};
    unsigned char buffer_32[4] = {0};
    int num = 0;

    buffer_16[1] = 'B';
    buffer_16[0] = 'M';
    header.type = buffer_16;
    
    fseek(fp, 0, SEEK_END);
    num = ftell(fp);
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    header.size = buffer_32;

    header.reserved1 = 0;
    header.reserved2 = 0;

    if (detail->type == bit_1) {
        num = 62;
    } else if (detail->type == bit_24) {
        num = 54;
    }
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    header.off_bits = buffer_32;

    num = 40;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.size = buffer_32;

    num = detail->width;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.width = buffer_32;
    
    num = detail->height;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.height = buffer_32;

    num = 1;
    sprintf(buffer_16, "%d", num);
    buffer_reverse(buffer_16);
    info.planes = buffer_16;

    if (detail->type == bit_1) {
        num = 1;
    } else if (detail->type == bit_24) {
        num = 24;
    }
    sprintf(buffer_16, "%d", num);
    buffer_reverse(buffer_16);
    info.bit_count = buffer_16;

    info.compression = 0;

    num = header.size - header.off_bits;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.size_image = buffer_32;

    num = 3780;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.x_pels_permeter = buffer_32;

    num = 3780;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    info.y_pels_permeter = buffer_32;

    info.clr_used = 0;

    info.clr_important = 0;

    write_file_data(fp, header, info, detail);
}

void write_image_data(FILE *fp, Detail *detail, void *data) {
    if (detail->type == bit_1) {
        Pixel_1 *data = (Pixel_1*)detail->data;
        fwrite(data, detail->width, 1, fp);
    } else if (detail->type == bit_24) {
        Pixel_24 *data = (Pixel_24*)detail->data;
        fwrite(data, sizeof(*data), 1, fp);
    }
    return;
}

int write_file_data(FILE *fp, bmp_file_header_t header, bmp_file_info_t info, Detail *detail) {
    fwrite(&header, sizeof(header), 1, fp);
    fwrite(&info, sizeof(info), 1, fp);
    unsigned char bit_1_palette[8] = {0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0};
    if (detail->type == bit_1) {
        fwrite(bit_1_palette, sizeof(bit_1_palette), 1, fp);
    }
    return OK;
}