#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "format.h"


void arg_fix(int *y, Detail *detail) {
    (*y) = detail->height - (*y);
    return;
}

void ini_image_data(FILE *fp, Detail *detail) {
    switch(detail->type) {
        case bit_1:
            detail->padding_in_bits = (32 - (detail->width * 1) % 32) % 32;
            detail->image_size = ((detail->width + detail->padding_in_bits) * detail->height) / 8;
            detail->padding_in_bytes = 0;  
            break;
            
        case bit_24:
            detail->padding_in_bytes = (4 - (detail->width * 3) % 4) % 4;
            detail->image_size = (detail->width * 3 + detail->padding_in_bytes) * detail->height;
            detail->padding_in_bits = 0;   
            break;
            
        default:
            printf("Error: unsupported image type\n");
            break;
    }
}

void get_a_pixel_color(uint8_t *dst_color, Detail *src_detail, FILE *fp_src, int pixel_offset) { 
    
    if (pixel_offset < 0 || pixel_offset >= src_detail->width *src_detail->height) {
        dst_color[0] = 0;
        dst_color[1] = 0;
        dst_color[2] = 0;
        return;
    }

    switch(src_detail->type) {
        case bit_1:
            uint32_t bit_offset = pixel_offset * 1;
            uint32_t byte_offset = bit_offset / 8;
            uint32_t bit_in_byte = bit_offset % 8;

            long old_pos = ftell(fp_src);

            fseek(fp_src, src_detail->image_offset + byte_offset, SEEK_SET);
            uint8_t buffer;
            fread(&buffer, 1, 1, fp_src);

            uint8_t pixel_bit = (buffer >> (7 - bit_in_byte)) & 0x01;

            dst_color[0] = pixel_bit ? 0xFF : 0x00;
            dst_color[1] = pixel_bit ? 0xFF : 0x00;
            dst_color[2] = pixel_bit ? 0xFF : 0x00;

            fseek(fp_src, old_pos, SEEK_SET);
            break;
        
        case bit_24 : {
            uint32_t bytes_per_row = src_detail->width * 3 + src_detail->padding_in_bytes;

            uint32_t row_start = (pixel_offset / src_detail->width) * bytes_per_row;

            uint32_t pixel_in_row = pixel_offset % src_detail->width;

            if (pixel_in_row >= src_detail->width) {
                dst_color[0] = 0;
                dst_color[1] = 0;
                dst_color[2] = 0;
                return;
            }

            uint32_t actual_offset = row_start + pixel_in_row * 3;

            long old_pos = ftell(fp_src);

            fseek(fp_src, src_detail->image_offset + actual_offset, SEEK_SET);
            uint8_t rgb[3];
            fread(rgb, 3, 1, fp_src);

            dst_color[0] = rgb[2];
            dst_color[1] = rgb[1];
            dst_color[2] = rgb[0];

            fseek(fp_src, old_pos, SEEK_SET);
            break;
        }

        default:
            dst_color[0] = 0;
            dst_color[1] = 0;
            dst_color[2] = 0;
            break;
    }
}
int get_bmp_data(FILE *fp_src, Detail *src_detail) {
    bmp_file_header_t header;
    bmp_file_info_t info;
    fread(&header, sizeof(header), 1, fp_src);
    fread(&info, sizeof(info), 1, fp_src);
    src_detail->width = info.width;
    src_detail->height = info.height;
    
    src_detail->type = (info.bit_count == 1) ? bit_1 : bit_24;
    src_detail->one_pixel_bit_size = info.bit_count;
    src_detail->image_size = info.size_image;
    src_detail->image_offset = header.off_bits;


    uint8_t *image_data = malloc((src_detail->image_size) * sizeof(uint8_t));
    fseek(fp_src, src_detail->image_offset, SEEK_SET);
    if (src_detail->type == bit_1) {
        fread(image_data, 1, src_detail->image_size, fp_src);
    } else if (src_detail->type == bit_24) {
        fread(image_data, 1, src_detail->image_size, fp_src);
    }
    src_detail->data = image_data;
    if (src_detail->data == NULL) printf("detail data log fail\n");
    ini_image_data(fp_src, src_detail);

    return OK;
}

void update_image_data(FILE *fp, Detail *detail, FILE *fp_src, Detail *src_detail, int paste_offset_x, int paste_offset_y) {
    if (detail->data != NULL) {
        free(detail->data);
        detail->data = NULL;
    }
    
    uint8_t *image_data = (uint8_t*)malloc(detail->image_size);
    if (image_data == NULL) return;
    
    memset(image_data, 0, detail->image_size);
    
    uint32_t bytes_per_row;
    if (detail->type == bit_1) {
        bytes_per_row = (detail->width + detail->padding_in_bits) / 8;
    } else { // bit_24
        bytes_per_row = detail->width * 3 + detail->padding_in_bytes;
    }
    
    for (int y = 0; y < detail->height; y++) {
        for (int x = 0; x < detail->width; x++) {
            uint8_t color[3] = {0};
            
            int is_in_paste_area = 0;
            if (fp_src != NULL && src_detail != NULL) {
                if (x >= paste_offset_x && x < paste_offset_x + src_detail->width &&
                    y >= paste_offset_y && y < paste_offset_y + src_detail->height) {
                    is_in_paste_area = 1;
                }
            }
            
            if (is_in_paste_area) {
                int src_x = x - paste_offset_x;
                int src_y = y - paste_offset_y;
                int src_pixel_index = src_y * src_detail->width + src_x;
                
                if (src_pixel_index >= 0 && src_pixel_index < src_detail->width * src_detail->height) {
                    get_a_pixel_color(color, src_detail, fp_src, src_pixel_index);
                } else {
                    memcpy(color, detail->bg_color, 3);
                }
            } else {
                memcpy(color, detail->bg_color, 3);
            }
            
            if (detail->type == bit_1) {
                uint32_t bit_pos = y * (detail->width + detail->padding_in_bits) + x;
                uint32_t byte_off = bit_pos / 8;
                uint32_t bit_in_byte = bit_pos % 8;
                
                uint8_t bit_value = (color[0] || color[1] || color[2]) ? 1 : 0;
                
                if (bit_value) {
                    image_data[byte_off] |= (0x80 >> bit_in_byte);
                } else {
                    image_data[byte_off] &= ~(0x80 >> bit_in_byte);
                }
            } else { // bit_24
                uint32_t byte_off = y * bytes_per_row + x * 3;
                image_data[byte_off] = color[2];     // B
                image_data[byte_off + 1] = color[1];  // G
                image_data[byte_off + 2] = color[0];  // R
            }
        } 
    }
    detail->data = image_data;
}

void write_image_data(FILE *fp, Detail *detail) {
    if (detail->data == NULL || detail->image_size == 0) return;
    
    fseek(fp, detail->image_offset, SEEK_SET);
    
    fwrite(detail->data, 1, detail->image_size, fp);
    
    fflush(fp);
}

int create_and_write_file_data(FILE *fp, Detail *detail) {

    bmp_file_header_t header;
    bmp_file_info_t info;

    memset(&header, 0, sizeof(header));
    memset(&info, 0, sizeof(info));

    header.type = 0x4D42;
    header.off_bits = (detail->type == bit_1) ? 62 : 54;
    
    info.size = 40;
    info.width = detail->width;
    info.height = detail->height;
    info.planes = 1;
    info.bit_count = (detail->type == bit_1) ? 1 : 24;
    info.compression = 0;
    info.size_image = detail->image_size;
    info.x_pels_permeter = 3780;
    info.y_pels_permeter = 3780;
    info.clr_used = 0;
    info.clr_important = 0;

    header.size = header.off_bits + detail->image_size;
    info.size_image = detail->image_size;

    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, fp);
    fwrite(&info, sizeof(info), 1, fp);
    
    if (detail->type == bit_1) {
        uint8_t palette[8] = {
            0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0x00
        };
        fwrite(palette, sizeof(palette), 1, fp);
    }

    return OK;
}

