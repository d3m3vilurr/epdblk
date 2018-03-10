#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define FB_DEV "/dev/graphics/fb0"

enum {
    NO_ERROR            = 0x00,
    ERROR_INV_ARGS      = 0x01,
    ERROR_OPEN_FB       = 0x02,
    ERROR_GET_FINFO     = 0x10,
    ERROR_GET_VINFO     = 0x11,
    ERROR_WRONG_FB      = 0x20,
    ERROR_FULL_REFRESH  = 0x30,
};

static char *fb_devs[] = {
    "/dev/graphics/fb0",
    "/dev/fb0",
    NULL,
};

static struct fb_fix_screeninfo fb_fix_info;
static struct fb_var_screeninfo fb_var_info;

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

#define MXCFB_SEND_UPDATE 0x4040462E
enum {
    WAVEFORM_MODE_AUTO  = 0x101,
};

enum {
    UPDATE_MODE_PARTIAL = 0x0,
    UPDATE_MODE_FULL    = 0x1,
};

struct mxcfb_rect {
    uint32_t top;
    uint32_t left;
    uint32_t width;
    uint32_t height;
};

struct mxcfb_alt_buffer_data {
    uint32_t phys_addr;
    uint32_t width;
    uint32_t height;
    struct mxcfb_rect alt_update_region;
};

struct mxcfb_update_data {
    struct mxcfb_rect update_region;
    uint32_t waveform_mode;
    uint32_t update_mode;
    uint32_t update_marker;
    int32_t temp;
    uint32_t flags;
    struct mxcfb_alt_buffer_data alt_buffer_data;
};

int refresh_imx(int fb) {

    if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_var_info) < 0) {
        return ERROR_GET_VINFO;
    }

    struct mxcfb_update_data update_data = {0};
    update_data.update_region.width = fb_var_info.xres;
    update_data.update_region.height = fb_var_info.yres;
    update_data.waveform_mode = WAVEFORM_MODE_AUTO;
    update_data.update_mode = UPDATE_MODE_FULL;
    update_data.temp = 0x18;

    if (ioctl(fb, MXCFB_SEND_UPDATE, &update_data) < 0) {
        return ERROR_FULL_REFRESH;
    }
    return NO_ERROR;
}

int main(int argc, char **argv) {
    uint8_t ret;

    if (argc < 2) {
        ret = ERROR_INV_ARGS;
        goto exit;
    }

    int sec = atoi(argv[1]);
    usleep(sec * 1000);

    int fb = open_framebuffer();
    if (fb < 0) {
        ret = ERROR_OPEN_FB;
        goto exit;
    }

    if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix_info) < 0) {
        ret = ERROR_GET_FINFO;
        goto cleanup;
    }

    if (strncmp(fb_fix_info.id, "mxc_epdc", 8) == 0) {
        ret = refresh_imx(fb);
    } else {
        ret = ERROR_WRONG_FB;
    }

cleanup:
    close_framebuffer(fb);
exit:
    return ret;
}
