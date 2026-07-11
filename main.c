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

FILE* open_new(const char *path, const char *name);
int analysis_command(const char *buffer, FILE *fp, Detail *detail, Detail *src_detail, _Bool *status);
void ini_detail(Detail *detail);

FILE* open_new(const char *path, const char *name) {
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
        printf("file not exist, created\n");
    } else if (isfound) {
        fp = fopen(full_path, "rb+");
        printf("file exist, load file\n");
    }
    closedir(dir);
    return fp;
}

int analysis_command(const char *buffer, FILE *fp, Detail *detail, Detail *src_detail, _Bool *status) {
    int argc = 0;
    char path[S] = {0};
    char buf_1[5] = {0};
    char color[3] = {0};
    int arg1, arg2, arg3, arg4 arg5= 0;
    argc = sscanf(buffer, "%s %s %d %d %d %d", buf_1, color, &arg1, &arg2, &arg3, &arg4, &arg5);
    if (argc < 1) return ERROR;
    if (strncmp(buf_1, "exit", 4) == 0 && strlen(buf_1) == 4) {
        (*status) = 0;
    } else if (strncmp(buf_1, "help", 4) == 0 && strlen(buf_1) == 4) {
        help();
    } else if (strncmp(buf_1, "show", 4) == 0 && strlen(buf_1) == 4) {
        show(detail);
    } else if (strncmp(buf_1, "clear", 5) == 0 && strlen(buf_1) == 5) {
        clear(detail, fp);
    } else if (strncmp(buf_1, "point", 5) == 0 && strlen(buf_1) == 5) {
        if (argc < 4) return ERROR;
        point(fp, detail, color, arg1, arg2, arg3);
        printf("point is write\n");
    } else if (strncmp(buf_1, "line", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        line(fp, detail, color, arg1, arg2, arg3, arg4, arg5);
        printf("line is write\n");
    } else if (strncmp(buf_1, "rect", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        rect(fp, detail, color, arg1, arg2, arg3, arg4, arg5);
        printf("write is write\n");
    } else if (strncmp(buf_1, "circle", 6) == 0 && strlen(buf_1) == 6) {
        if (argc < 5) return ERROR;
        circle(fp, detail, color, arg1, arg2, arg3, arg5);
        printf("circle is write\n");
    } else if (strncmp(buf_1, "paste", 5) == 0 && strlen(buf_1) == 5) {
        if (argc < 4) return ERROR;
        printf("input path : \n");
        scanf("%s", path);
        paste(fp, detail, src_detail, arg1, arg2, arg3, path);
        printf("file is paste\n");
    } else {
        return ERROR;
    }
    write_image_data(fp, detail);
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

int main(int argc, const char *argv[]) {
    if (argc < 7) {
        printf("usage : <filePath> <fileName> <width> <height> <depth> <background color(0x b,g,r)>\n");
        return ERROR;
    }

    FILE *fp = open_new(argv[1], argv[2]);
    if (fp == NULL) return ERROR;

    Detail file_detail;
    Detail src_detail_file_detail;

    Detail *detail = &file_detail;
    Detail *src_detail = &src_detail_file_detail;
    ini_detail(detail);
    ini_detail(src_detail);

    detail->width = atoi(argv[3]);
    detail->height = atoi(argv[4]);
    strncpy(detail->bg_color, argv[6], 3);

    if (strncmp(argv[5], "1", 1) == 0) {
        detail->type = bit_1;
        detail->one_pixel_bit_size = 1;
        detail->image_offset = 62;
    }else if (strncmp(argv[5], "24", 2) == 0) {
        detail->type = bit_24;
        detail->one_pixel_bit_size = 24;
        detail->image_offset = 54;
    } else {
        printf("depth support : [1] [24]\n");
    }

 
    ini_image_data(fp, detail);  // ini padding in detail
    
    detail->data = update_image_data(fp, detail, NULL, NULL);
    write_image_data(fp, detail);
    create_and_write_file_data(fp, detail);

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
}