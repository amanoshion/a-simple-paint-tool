#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include "format.h"
#include "operation.h"

#define S 32
#define M 64
#define L 128
#define XL 256
#define XXL 512

FILE* open_new(_Bool *flag, const char *path, const char *name);
int analysis_command(const char *buffer, FILE *fp, Detail *detail, Detail *src_detail, _Bool *status);
void ini_detail(Detail *detail);
int rgbstring_to_bgruint8(char *str, uint8_t *bgr);

FILE* open_new(_Bool *flag, const char *path, const char *name) {
    DIR *dir = opendir(path);
    if (dir == NULL) return NULL;
    struct dirent *entry = NULL;
    _Bool isfound = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, name) == 0 && (strlen(entry->d_name) == strlen(name))) {
            isfound = 1;
        } 
    }
    char full_path[M] = {0};
    int l;
    l = strlen(path);
    
    if (path[l-1] == '\n') {
        l--;
    }

    snprintf(full_path, sizeof(full_path), "%s%s", path, name);
    
    FILE *fp = NULL;
    if (!isfound) {
        fp = fopen(full_path, "wb+");
        (*flag) = 0;
        printf("file not exist, created\n");
    } else if (isfound) {
        fp = fopen(full_path, "rb+");
        (*flag) = 1;
        printf("file exist, load file\n");
    }
    closedir(dir);
    return fp;
}

int analysis_command(const char *buffer, FILE *fp, Detail *detail, Detail *src_detail, _Bool *status) {
    int argc = 0;
    char buf_1[S] = {0};
    uint8_t color[3] = {0};
    char tmp_color[S] = {0};
    int arg1, arg2, arg3, arg4, arg5 = 0;
    
    argc = sscanf(buffer, "%s %s %d %d %d %d %d", buf_1, tmp_color, &arg1, &arg2, &arg3, &arg4, &arg5);
    if (argc < 1) return ERROR;
    if (argc >= 2) {
        if (strcmp(tmp_color, "red") == 0) {
            color[0] = 0xFF;
            color[1] = 0x00;
            color[2] = 0x40;
        } else if (strcmp(tmp_color, "green") == 0) {
            color[0] = 0x00;
            color[1] = 0xFF;
            color[2] = 0x6F;
            
        } else if (strcmp(tmp_color, "blue") == 0) {
            color[0] = 0x00;
            color[1] = 0x88;
            color[2] = 0xFF;
        } else {
            rgbstring_to_bgruint8(tmp_color, color);
        }
    }
    if (strncmp(buf_1, "exit", 4) == 0 && strlen(buf_1) == 4) {
        (*status) = 0;
        return OK;
    } else if (strncmp(buf_1, "help", 4) == 0 && strlen(buf_1) == 4) {
        help();
        return OK;
    } else if (strncmp(buf_1, "show", 4) == 0 && strlen(buf_1) == 4) {
        show(detail);
        return OK;
    } else if (strncmp(buf_1, "clear", 5) == 0 && strlen(buf_1) == 5) {
        clear(detail, fp);
    } else if (strncmp(buf_1, "point", 5) == 0 && strlen(buf_1) == 5) {
        if (argc < 4) return ERROR;
        arg_fix(&arg3, detail);
        point(fp, detail, color, arg1, arg2, arg3);
    } else if (strncmp(buf_1, "line", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        arg_fix(&arg3, detail);
        arg_fix(&arg5, detail);
        line(fp, detail, color, arg1, arg2, arg3, arg4, arg5);
    } else if (strncmp(buf_1, "rect", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        arg_fix(&arg3, detail);
        arg_fix(&arg5, detail);
        rect(fp, detail, color, arg1, arg2, arg3, arg4, arg5);
    } else if (strncmp(buf_1, "circle", 6) == 0 && strlen(buf_1) == 6) {
        if (argc < 5) return ERROR;
        arg_fix(&arg3, detail);
        circle(fp, detail, color, arg1, arg2, arg3, arg4);
    } else if (strncmp(buf_1, "paste", 5) == 0 && strlen(buf_1) == 5) {
        if (argc < 1) return ERROR;
        char path[S] = {0};
        char name[S] = {0};
        _Bool isexist;
        int x, y;
        printf("input path : \n");
        scanf("%s", path);
        printf("input file name : \n");
        scanf("%s", name);
        printf("input start x :");
        scanf("%d", &x);
        printf("input start y :");
        scanf("%d", &y);
        
        arg_fix(&y, detail);
        FILE *fp_src = open_new(&isexist, path, name);
        if (fp_src == NULL) return ERROR;
        paste(fp, detail, fp_src, src_detail, x, y);
    } else {
        printf("cannot analysis\n");
        return ERROR;
    }
    printf("========================================\n");
    printf("file is updated\n");
    return OK;
}

void ini_detail(Detail *detail) {
    detail->width = 0;
    detail->height = 0;
    detail->padding_in_bits = 0;
    detail->padding_in_bytes = 0;
    detail->type = undefined;
    detail->one_pixel_bit_size = 0;
    detail->image_size = 0;
    detail->image_offset = 0;
    memset(detail->bg_color, 0, 3);
    detail->data = NULL;
}

int rgbstring_to_bgruint8(char *str, uint8_t *bgr) {
    int len = strlen(str);
    if (len < 6) return ERROR;
    char str_fix[6] = {0};
    if ((str[len - 1]) == '0' && (str[len - 2]) == 'x' && len == 8) {
        strncpy(str_fix, str + 2, 6);
    } else if (len == 6){
        strncpy(str_fix, str, 6);
    } else {
        return ERROR;
    }
    unsigned int tmp;
    sscanf(str_fix, "%x", &tmp);  // 
    
    bgr[0] = 0;
    bgr[1] = 0;
    bgr[2] = 0;
    bgr[0] |= tmp & 0xFF;
    bgr[1] |= tmp>>8 & 0xFF;
    bgr[2] |= tmp>>16 & 0xFF;
    return OK;
}

int main(int argc, const char *argv[]) {
    if (argc < 7) {
        printf("usage : <filePath> <fileName> <width> <height> <depth> <background color [red|green|blue|yellow]>\n");
        return ERROR;
    }

    _Bool file_exist = 0;
    FILE *fp = open_new(&file_exist, argv[1], argv[2]);
    if (fp == NULL) return ERROR;

    Detail file_detail;
    Detail src_detail_file_detail;

    Detail *detail = &file_detail;
    Detail *src_detail = &src_detail_file_detail;
    ini_detail(detail);
    ini_detail(src_detail);

    if (!file_exist) {
        detail->width = atoi(argv[3]);
        detail->height = atoi(argv[4]);
        if (strncmp(argv[5], "1", 1) == 0) {
            detail->type = bit_1;
            detail->one_pixel_bit_size = 1;
            detail->image_offset = 62;
        } else if (strncmp(argv[5], "24", 2) == 0) {
            detail->type = bit_24;
            detail->one_pixel_bit_size = 24;
            detail->image_offset = 54;
        } else {
            printf("depth support : [1] [24]\n");
        }
        char tmp[S] = {0};
        memcpy(tmp, argv[6], strlen(argv[6]));
        if (!rgbstring_to_bgruint8(tmp, detail->bg_color)) return ERROR;

        ini_image_data(fp, detail);  // ini padding in detail
        update_image_data(fp, detail, NULL, NULL, 0, 0);
        create_and_write_file_data(fp, detail);
        write_image_data(fp, detail);
    } else {
        get_bmp_data(fp, detail);
        char tmp[S] = {0};
        memcpy(tmp, argv[6], strlen(argv[6]));
        rgbstring_to_bgruint8(tmp, detail->bg_color);
    }
    


    help();
    char command_buffer[M] = {0};
    _Bool isContinue = 1;
    while(isContinue) {
        memset(command_buffer, 0, M);
        fgets(command_buffer, M, stdin);
        analysis_command(command_buffer, fp, detail, src_detail, &isContinue);
    }
    if (detail != NULL && detail->data != NULL) {
        free(detail->data);
        detail->data = NULL;
    }
    if (src_detail != NULL && src_detail->data != NULL) {
        free(src_detail->data);
        src_detail->data = NULL;
    }
    return 0;
}