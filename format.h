#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

typedef enum {
    bit_1,
    bit_4,
    bit_8,
    bit_16,
    bit_24,
    bit_32
} Deep_Type;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} RGBQUAD;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Pixel_24;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t alpha;
} Pixel_32;

typedef struct {
        uint16_t type;  //='B' 'M'
        uint32_t size;  //file's total byte size
        uint16_t reserved1; //=0
        uint16_t reserved2; //=0
        uint32_t off_bits;  //[photo pixel data's offset] 
        //-> 1 : 14 + 40 + 2*4 = 62, 4 : 14 + 40 + 16*4 = 118
        //-> 8 : 14 + 40 + 256*4 = 1078, 16/24/32 : 54
                            
} bmp_file_header_t;    //size : 14 byte

typedef struct {
        uint32_t size; //=40
        int32_t width;  
        int32_t height;
        uint16_t planes;    //=1
        uint16_t bit_count;     // depth
        uint32_t compression;   //=0
        uint32_t size_image;    // image data's byte size
        uint32_t x_pels_permeter;   //=3780
        uint32_t y_pels_permeter;   //=3780
        uint32_t clr_used;      //=0
        uint32_t clr_important; //=0
} bmp_file_info_t;    //size : 40 byte

void ini_palete(char *data, RGBQUAD *palete, int limit);
void make_palete(Deep_Type type);

#endif