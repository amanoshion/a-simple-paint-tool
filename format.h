#ifndef FORMAT_H
#define FORMAT_H

#include <stdio.h>
#include <stdint.h>
#define ERROR -1
#define OK 1

typedef enum {
    bit_1,
    bit_24,
    undefined
} Deep_Type;

typedef struct {
    uint32_t bits;   
} Pixel_1;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Pixel_24;

typedef struct Detail {
    int width;  //pixel num
    int height;
    int padding;
    Deep_Type type;
    int one_pixel_bit_size;
    int image_size;
    int image_offset;
    unsigned char bg_color[3];
    void *data;
} Detail;

typedef struct {
        unsigned char type[2];  //='B' 'M'
        unsigned char size[4];  // 14 + 40 + palette size + size_image + padding size
        unsigned char reserved1[2]; //=0
        unsigned char reserved2[2]; //=0
        unsigned char off_bits[4];  //[photo pixel data's offset] 
        //-> 1 : 14 + 40 + 2*4 = 62, 4 : 14 + 40 + 16*4 = 118
        //-> 8 : 14 + 40 + 256*4 = 1078, 16/24/32 : 54
                            
} bmp_file_header_t;    //size : 14 byte

typedef struct {
        unsigned char size[4]; //=40
        unsigned char width[4];  // pixel num only
        unsigned char height[4];  // pixel num only
        unsigned char planes[2];    //=1
        unsigned char bit_count[2];     // depth
        unsigned char compression[4];   //=0
        unsigned char size_image[4];    // image data's byte size, including padding size
        unsigned char x_pels_permeter[4];   //=3780
        unsigned char y_pels_permeter[4];   //=3780
        unsigned char clr_used[4];      //=0
        unsigned char clr_important[4]; //=0
} bmp_file_info_t;    //size : 40 byte

int ini_image_data(FILE *fp, Detail *detail);
void get_a_pixel_color(unsigned char *dst_color, Detail *src_detail, FILE *fp_src, int pixel_offset);

void *update_image_data(FILE *fp, Detail *detail, Detail *src_detail, FILE *fp_src);
void write_image_data(FILE *fp, Detail *detail);

void buffer_reverse(unsigned char *buffer);
int write_file_data(FILE *fp, bmp_file_header_t header, bmp_file_info_t info, Detail *detail);
int create_and_write_file_data(FILE *fp, Detail *detail);

#endif