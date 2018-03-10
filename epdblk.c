#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define FB_DEV "/dev/graphics/fb0"

static char *fb_devs[] = {
    "/dev/graphics/fb0",
    "/dev/fb0",
    NULL,
};

static struct fb_var_screeninfo fb_info;

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

int open_framebuffer() {
    int fb;
    int i = 0;
    while (fb_devs[i]) {
        fb = open(fb_devs[i], O_RDWR);
        if (fb >= 0) {
            return fb;
        }
        i++;
    }
    return -1;
}

int close_framebuffer(int fb) {
    close(fb);
}

int main(int argc, char **argv) {
    int ret = 0;

    if (argc < 2) {
        ret = -1;
        goto exit;
    }

    int sec = atoi(argv[1]);
    usleep(sec * 1000);

    int fb = open_framebuffer();
    if (fb < 0) {
        ret = -2;
        goto exit;
    }

    if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_info) < 0) {
        ret = -3;
        goto cleanup;
    }

    struct update_payload_t payload = {
        .top = 0,
        .left = 0,
        .width = fb_info.xres,
        .height = fb_info.yres,
        .waveform_mode = 0x101,
        .update_mode = 1,
        .temp = 0x18,
        .flags = 0,
    };

    if (ioctl(fb, 0x4040462E, &payload) < 0) {
        ret = -4;
        goto cleanup;
    }

cleanup:
    close_framebuffer(fb);
exit:
    return ret;
}
