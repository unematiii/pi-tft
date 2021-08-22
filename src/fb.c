#include "fb.h"

RGB565_Color RGBToRGB565(RGB_Color color) {
    uint16_t b = (color.blue >> 3) & 0x1f;
    uint16_t g = ((color.green >> 2) & 0x3f) << 5;
    uint16_t r = ((color.red >> 3) & 0x1f) << 11;

    return (RGB565_Color) (r | g | b);
}

RGB_Color RGB565ToRGB(RGB565_Color color) {
    RGB_Color rgb = {0};

    rgb.red = ((color >> 11) & 0x3f) << 3;
    rgb.green = ((color & 0x07c0) >> 5) << 2;
    rgb.blue = (color & 0x3f) << 3;

    return rgb;
}

int InitFb(const char* dev, Fb_Data* fb) {
    fb->fd = open(dev, O_RDWR);

    if (fb->fd >= 0) {
        struct fb_var_screeninfo vinfo;

        ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo);

        fb->width = vinfo.xres;
        fb->height = vinfo.yres;
        fb->bytes_per_pixel = vinfo.bits_per_pixel / 8;
        fb->size = fb->width * fb->height * fb->bytes_per_pixel;

        fb->buffer = (char *) mmap(0, fb->size,
                PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, (off_t) 0);

        return 0;
    }

    return -1;
}

int DestroyFb(Fb_Data* fb) {
    munmap(fb->buffer, fb->size);
    close(fb->fd);

    return 0;
}

void DrawRect(int x, int y, int w, int h, bool fill, RGB565_Color color, Fb_Data* fb) {
    int x1 = x + w;
    int y1 = y + h;

    if (x1 > fb->width - 1)
        x1 = fb->width - 1;

    if (y1 > fb->height - 1)
        y1 = fb->height - 1;

    for (int x0 = x; x0 <= x1; x0++) {
        for (int y0 = y; y0 <= y1; y0++) {
            if (x0 == x || y0 == y || y0 == y1 || x0 == x1) {
                SetPixelAt(x0, y0, color, fb);
                continue;
            }

            if (fill) {
                SetPixelAt(x0, y0, color, fb);
            }
        }
    }
}

RGB565_Color GetPixelAt(int x, int y, Fb_Data* fb) {
    int offset = (y * fb->width + x) * fb->bytes_per_pixel;
    RGB565_Color pixel = 0;

    memcpy((void *) &pixel, (void *) (fb->buffer + offset), sizeof (RGB565_Color));

    return pixel;
}

void SetPixelAt(int x, int y, RGB565_Color color, Fb_Data* fb) {
    int offset = (y * fb->width + x) * fb->bytes_per_pixel;
    memcpy((void *) (fb->buffer + offset), &color, sizeof (color));
}
