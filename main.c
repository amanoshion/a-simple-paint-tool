#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include "format.h"
#define ERROR -1
#define OK 1

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

void help() {
    printf("========================================\n");
    printf("=\tpaint command\n");
    printf("================basic===================\n");
    printf("exit\n");
    printf("help\n");
    printf("show\n");
    printf("================main====================\n");
    printf("clear\n");
    printf("point + <pixelX> <pixelY> <color>\n");
    printf("line + <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end> <color>\n");
    printf("rect + <pixelX_start> <pixelY_start> <pixelX_end> <pixelY_end> <color>\n");
    printf("circle + <pixelX> <pixelY> <radius> <color>\n");
    printf("========================================\n");

}
int main(int argc, const char *argv[]) {
    if (argc < 7) {
        printf("usage : <filePath> <fileName> <width> <height> <depth> <background color>\n");
        return ERROR;
    }

    FILE *fp = open_new(argv[1], argv[2]);
    if (fp == NULL) return ERROR;

    Deep_Type type = -1;
    if (strncmp(argv[6], "1", strlen(argv[6])) == 0) type = bit_1;
    if (strncmp(argv[6], "4", strlen(argv[6])) == 0) type = bit_4;
    if (strncmp(argv[6], "8", strlen(argv[6])) == 0) type = bit_8;
    if (strncmp(argv[6], "16", strlen(argv[6])) == 0) type = bit_16;
    if (strncmp(argv[6], "24", strlen(argv[6])) == 0) type = bit_24;
    if (strncmp(argv[6], "32", strlen(argv[6])) == 0) type = bit_32;
    if (type == -1) return ERROR;

    make_palete(type);
    ini_image_data(type);

    help();
    char command_buffer[M] = {0};
    _Bool isContinue = 1;
    while(isContinue) {
        memset(command_buffer, 0, M);
        scanf("%s", command_buffer);
        analysis_command(command_buffer);
        
        
        
    }
}