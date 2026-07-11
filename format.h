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
    uint32_t width;  //pixel num
    uint32_t height;
    int padding_in_bits;
    int padding_in_bytes;
    Deep_Type type;
    int one_pixel_bit_size;
    int image_size; // bytes num    // TODO : fix image_size to bytes , not pixel num
    int image_offset;
    char bg_color[3];
    void *data;
} Detail;

#pragma pack(push, 1)
typedef struct {
        uint16_t type;  //='B' 'M'
        uint32_t size;  // 14 + 40 + palette size + size_image + padding size
        uint16_t reserved1; //=0
        uint16_t reserved2; //=0
        uint32_t off_bits;  //[photo pixel data's offset] 
        //-> 1 : 14 + 40 + 2*4 = 62, 4 : 14 + 40 + 16*4 = 118
        //-> 8 : 14 + 40 + 256*4 = 1078, 16/24/32 : 54
                            
} bmp_file_header_t;    //size : 14 byte

typedef struct {
        uint32_t size; //=40
        uint32_t width;  // pixel num only
        uint32_t height;  // pixel num only
        uint16_t planes;    //=1
        uint16_t bit_count;     // depth
        uint32_t compression;   //=0
        uint32_t size_image;    // image data's byte size, including padding size
        uint32_t x_pels_permeter;   //=3780
        uint32_t y_pels_permeter;   //=3780
        uint32_t clr_used;      //=0
        uint32_t clr_important; //=0
} bmp_file_info_t;    //size : 40 byte
#pragma pack(pop)

void ini_image_data(FILE *fp, Detail *detail);
void get_a_pixel_color(char *dst_color, Detail *src_detail, FILE *fp_src, int pixel_offset);

void *update_image_data(FILE *fp, Detail *detail, Detail *src_detail, FILE *fp_src);
void write_image_data(FILE *fp, Detail *detail);

int create_and_write_file_data(FILE *fp, Detail *detail);

#endif