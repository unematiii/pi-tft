#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifndef PITFT_FB_H
#define PITFT_FB_H

typedef struct {
    int fd;
    int bytes_per_pixel;
    int width;
    int height;
    int size;
    char* buffer;
} Fb_Data;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB_Color;

typedef uint16_t RGB565_Color;

RGB565_Color RGBToRGB565(RGB_Color color);
RGB_Color RGB565ToRGB(RGB565_Color color);

int InitFb(const char* dev, Fb_Data* fb);
int DestroyFb(Fb_Data* fb);

void DrawRect(int x, int y, int w, int h, bool fill, RGB565_Color color, Fb_Data* fb);
RGB565_Color GetPixelAt(int x, int y, Fb_Data* fb);
void SetPixelAt(int x, int y, RGB565_Color color, Fb_Data* fb);

#endif /* PITFT_FB_H */
