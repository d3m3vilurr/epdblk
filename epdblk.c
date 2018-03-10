#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define FB_DEV "/dev/graphics/fb0"

struct display_t {
    uint32_t width;
    uint32_t height;
};

struct update_payload_t {
    uint32_t top;
    uint32_t left;
    uint32_t width;
    uint32_t height;
    uint32_t waveform_mode;
    uint32_t update_mode;
    uint32_t update_marker;
    uint32_t temp;
    uint32_t flags;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    int sec = atoi(argv[1]);
    usleep(sec * 1000);

    int fb = open(FB_DEV, O_RDWR);
    if (fb < 0) {
        return -2;
    }

    struct display_t display;
    if (ioctl(fb, 0x4600, &display) < 0) {
        close(fb);
        return -3;
    }

    struct update_payload_t payload = {
        .top = 0,
        .left = 0,
        .width = display.width,
        .height = display.height,
        .waveform_mode = 0x101,
        .update_mode = 1,
        .temp = 0x18,
        .flags = 0,
    };

    if (ioctl(fb, 0x4040462E, &payload) < 0) {
        close(fb);
        return -4;
    }

    close(fb);
}
