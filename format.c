#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "format.h"

int ini_image_data(FILE *fp, Detail *detail) {
    int padding = 0;
    int p_size = 0;
    switch(detail->type) {
        case bit_1:
            p_size = detail->one_pixel_bit_size;
            break;
        case bit_24:
            p_size = detail->one_pixel_bit_size;
            break;
    }
    int ret;
    if ((ret = ((p_size*detail->width) % 32)) != 0) {
        padding = 32 - ret;
    }
    return padding;
}

void get_a_pixel_color(unsigned char *dst_color, Detail *src_detail, FILE *fp_src, int pixel_offset) { // the pixel_offset start from 1, not 0
    
    unsigned char color[3] = {0};
    unsigned char color_bit_1;
    switch(src_detail->type) {
        case bit_1:
            int ret;
            int bits;
            int bits_end;
            int bits_nail;
            int bytes;
            unsigned char buffer[1] = {0};
            bits = (pixel_offset / (src_detail->width + src_detail->padding)) * src_detail->width;//  total pixels of head lines
            bits_end = pixel_offset - bits;             //  end pixels of the end line
            if (bits_end > src_detail->width * 8) {
                memset(color, 0, 3);
                strncpy(dst_color, color, 3);
            }  // if target pixel is on padding , return 0
            bits_nail = bits_end % 8;

            bytes = bits/8 + bits_end/8;
            bits_nail = bits_end % 8;
            
            fseek(fp_src, src_detail->image_offset + bytes, SEEK_SET);
            fread(buffer, sizeof(uint8_t), 1, fp_src);
            buffer_reverse(buffer);
            buffer[0] >>= bits_nail;
            if (bits_nail & 1) {
                sprintf(color, "%d", 1);
            } else {
                sprintf(color, "%d", 0);
            }
            strncpy(dst_color, color, 3);
            break;

        case bit_24:
            int bytes_2 = pixel_offset*3;
            if (bytes % (src_detail->width + src_detail->padding) > src_detail->width) {
                memset(color, 0, 3);
                strncpy(dst_color, color, 3);
                break;
            }  // if target pixel is on padding , return 0
            unsigned char buffer_2[3] = {0};
            fread(buffer_2, sizeof(uint8_t), 1, fp_src);
            strncpy(color, buffer_2, 3);
            strncpy(dst_color, color, 3);
            break;
    }
}
void *update_image_data(FILE *fp, Detail *detail,   // updating file's args
    Detail *src_detail, FILE *fp_src) {  // the reading-from file's args, the pixel_offset start from 1, not 0
    unsigned char color[3] = {0};
    int pixel_offset = 1;
    switch(detail->type) {
        case bit_1:
            Pixel_1 *data_1 = malloc(detail->image_size*sizeof(Pixel_1));
            unsigned char pixel_buffer = 0;
            unsigned char color_bit_1;

            int k = 0;
            for (int j = 0; j < detail->height; j++) {
                for(int i = 0; i < detail->width + detail->padding; i++) {
                    if (src_detail == NULL || fp_src == NULL || pixel_offset == 0) {
                        strncpy(color, detail->bg_color, 3);   //get pixel_color
                    } else {
                        get_a_pixel_color(color, src_detail, fp_src, pixel_offset++);   //get pixel_color

                    }
                    color_bit_1 = (color[3]&1);
                    k++;
                    pixel_buffer <<= 1;
                    if (i < detail->width) {
                        pixel_buffer |= color_bit_1; 
                    } else {
                        pixel_buffer |= 0;
                    }
                    if (k >= 7) {
                        data_1[i].bits = pixel_buffer;
                        k = 0;
                    }
                }
            } 
            return data_1;
            break;
        case bit_24:
            Pixel_24 *data_24 = malloc(detail->image_size*sizeof(Pixel_24));

            int i = 0;
            for (int j = 0; j < detail->height; j++) {
                for(i = 0; i < detail->width; i++) {
                    if (src_detail == NULL || fp_src == NULL || pixel_offset == 0) {
                        strncpy(color, detail->bg_color, 3);   //get pixel_color
                    } else {
                        get_a_pixel_color(color, src_detail, fp_src, pixel_offset++);   //get pixel_color

                    }
                    data_24[i].b = color[0];
                    data_24[i].g = color[1];
                    data_24[i].r = color[2];
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
    free(tmp);
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
    strncpy(header.type, buffer_16, 2);
    
    fseek(fp, 0, SEEK_END);
    num = ftell(fp);
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(header.size, buffer_32, 4);
    
    memset(header.reserved1, 0, 2);
    memset(header.reserved2, 0, 2);
    
    if (detail->type == bit_1) {
        num = 62;
    } else if (detail->type == bit_24) {
        num = 54;
    }
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(header.off_bits, buffer_32, 4);
    
    num = 40;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(header.size, buffer_32, 4);
    
    num = detail->width;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(info.width, buffer_32, 4);
    
    num = detail->height;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(info.height, buffer_32, 4);
    
    num = 1;
    sprintf(buffer_16, "%d", num);
    buffer_reverse(buffer_16);
    strncpy(info.planes, buffer_16, 2);
    
    if (detail->type == bit_1) {
        num = 1;
    } else if (detail->type == bit_24) {
        num = 24;
    }
    sprintf(buffer_16, "%d", num);
    buffer_reverse(buffer_16);
    strncpy(info.bit_count, buffer_16, 2);
    
    memset(info.compression, 0, 4);
    
    num = header.size - header.off_bits;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(info.size_image, buffer_32, 4);
    
    num = 3780;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(info.x_pels_permeter, buffer_32, 4);
    
    num = 3780;
    sprintf(buffer_32, "%d", num);
    buffer_reverse(buffer_32);
    strncpy(info.y_pels_permeter, buffer_32, 4);

    memset(info.clr_used, 0, 4);
    memset(info.clr_important, 0, 4);

    write_file_data(fp, header, info, detail);
}

void write_image_data(FILE *fp, Detail *detail) {
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