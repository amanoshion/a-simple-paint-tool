#include <stdio.h>

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
    strncpy(full_path, path, l);
    strncpy(full_path + l, name, strlen(name));
    
    FILE *fp = NULL;
    if (!isfound) {
        fp = fopen(full_path, "wb+");
        printf("file not exist , created\n");
    } else if (isfound) {
        fp = fopen(full_path, "rb+");
        printf("file exist , load file\n");
    }
    closedir(dir);
    return fp;
}

int analysis_command(const char *buffer, FILE *fp, Detail *detail) {
    int argc = 0;
    unsigned buf_1[5] = {0};
    unsigned char color[3] = {0};
    int pos1_x, pos1_y = 0;
    int pos2_x, pos2_y = 0;
    argc = sscanf(buffer, "%s %s %d %d %d %d", buf_1, color, pos1_x, pos1_y, pos2_x, pos2_y);
    if (argc < 1) return ERROR;
    if (strncmp(buf_1, "exit", 4) == 0 && strlen(buf_1) == 4) {
        exit();
    } else if (strncmp(buf_1, "help", 4) == 0 && strlen(buf_1) == 4) {
        help();
    } else if (strncmp(buf_1, "show", 4) == 0 && strlen(buf_1) == 4) {
        show();
    } else if (strncmp(buf_1, "clear", 5) == 0 && strlen(buf_1) == 5) {
        clear();
    } else if (strncmp(buf_1, "point", 5) == 0 && strlen(buf_1) == 5) {
        if (argc < 4) return ERROR;
        point(fp, detail, color, pos1_x, pos1_y);
    } else if (strncmp(buf_1, "line", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        line();
    } else if (strncmp(buf_1, "rect", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 6) return ERROR;
        rect();
    } else if (strncmp(buf_1, "line", 4) == 0 && strlen(buf_1) == 4) {
        if (argc < 5) return ERROR;
        circle();
    } else {
        return ERROR;
    }
}
void help() {
    printf("========================================\n");
    printf("=\tpaint command\n");
    printf("================basic===================\n");
    printf("exit\n");
    printf("help\n");
    printf("show\n");
    printf("================main====================\n");
    printf("clear\n");
    printf("point + <color> <pixelX> <pixelY>\n");
    printf("line + <color> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("rect + <color> <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end>\n");
    printf("circle + <color> <pixelX> <pixelY> <radius>\n");
    printf("========================================\n");

}
int main(int argc, const char *argv[]) {
    if (argc < 7) {
        printf("usage : <filePath> <fileName> <width> <height> <depth> <background color>\n");
        return ERROR;
    }

    FILE *fp = open_new(argv[1], argv[2]);
    if (fp == NULL) return ERROR;

    Detail file_detail;
    file_detail.width = 0;
    file_detail.height = 0;
    file_detail.padding = 0;
    file_detail.one_piexel_bit_size = 0;
    file_detail.data = NULL;

    Detail *detail = &file_detail;
    
    detail->width = atoi(argv[3]);
    detail->height = atoi(argv[4]);
    strncpy(detail->color, argv[6], 3);
    Deep_Type type = -1;
    if (strncmp(argv[6], "1", strlen(argv[6])) == 0) {
        detail->type = bit_1;
        detail->one_piexel_bit_size = 1;
        detail->image_offset = 62;
    }else if (strncmp(argv[6], "24", strlen(argv[6])) == 0) {
        detail->type = bit_24;
        detail->one_piexel_bit_size = 24;
        detail->image_offset = 54;
    } 
    if (type == -1) return ERROR;

    int padding = ini_image_data(fp, detail);
    detail->padding = padding;
    detail->pixel_num = (detail->width + detail->padding)*detail->height;
    

    detail->data = create_image_data(fp, detail);
    write_image_data(fp, detail, detail->data);

    create_and_write_file_data(fp, detail);

    help();
    char command_buffer[M] = {0};
    _Bool isContinue = 1;
    while(isContinue) {
        memset(command_buffer, 0, M);
        scanf("%s", command_buffer);
        analysis_command(command_buffer, fp, detail);
        
               
    }
}