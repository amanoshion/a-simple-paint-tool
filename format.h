#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>
#define ERROR -1
#define OK 1

typedef enum {
    bit_1,
    bit_24
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
    int one_piexel_bit_size;
    int pixel_num;
    int image_offset;
    unsigned char color[3];
    void *data;
} Detail;

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
        int32_t width;  // pixel num only
        int32_t height;  // pixel num only
        uint16_t planes;    //=1
        uint16_t bit_count;     // depth
        uint32_t compression;   //=0
        uint32_t size_image;    // image data's byte size, including padding size
        uint32_t x_pels_permeter;   //=3780
        uint32_t y_pels_permeter;   //=3780
        uint32_t clr_used;      //=0
        uint32_t clr_important; //=0
} bmp_file_info_t;    //size : 40 byte

int create_image_data(FILE *fp, Detail *detail);
void write_image_data(FILE *fp, Detail *detail, void *data);

void buffer_reverse(unsigned char *buffer);
int write_file_data(FILE *fp, bmp_file_header_t header, bmp_file_info_t info, Detail *detail);
int create_and_write_file_data(FILE *fp, Detail *detail);

#endif