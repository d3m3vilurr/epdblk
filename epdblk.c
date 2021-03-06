#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "mxcfb.h"
#include "ebc.h"
#include <stdio.h>

enum {
    NO_ERROR            = 0x00,
    ERROR_INV_ARGS      = 0x01,
    ERROR_OPEN_FB       = 0x02,
    ERROR_OPEN_EBC      = 0x03,
    ERROR_GET_FINFO     = 0x10,
    ERROR_GET_VINFO     = 0x11,
    ERROR_WRONG_FB      = 0x20,
    ERROR_MXCFB_UPDATE  = 0x30,
    ERROR_EBC_GET_BUF   = 0x40,
    ERROR_EBC_SET_BUF   = 0x41,
    ERROR_MMAP          = 0xF0,
};

enum {
    REFRESH_WAVEFORM    = 0x0,
    REFRESH_FULL        = 0x1,
};

static char *fb_devs[] = {
    "/dev/graphics/fb0",
    "/dev/fb0",
    NULL,
};

static char *ebc_devs[] = {
    "/dev/ebc",
    NULL,
};

static struct fb_fix_screeninfo fb_fix_info;
static struct fb_var_screeninfo fb_var_info = {0};

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
    return close(fb);
}

int open_ebc() {
    int ebc;
    int i = 0;
    while (ebc_devs[i]) {
        ebc = open(ebc_devs[i], O_RDWR);
        if (ebc >= 0) {
            return ebc;
        }
        i++;
    }
    return -1;
}

int close_ebc(int ebc) {
    return close(ebc);
}

int refresh_mxcfb(int fb, int mode) {
    if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_var_info) < 0) {
        return ERROR_GET_VINFO;
    }

    struct mxcfb_update_data update_data = {0};
    struct mxcfb_update_marker_data marker_data;

    update_data.update_region.width = fb_var_info.xres;
    update_data.update_region.height = fb_var_info.yres;
    update_data.update_mode = MXCFB_UPDATE_MODE_FULL;
    update_data.temp = MXCFB_TEMP_USE_REMARKABLE_DRAW;

    if (mode == REFRESH_FULL) {
        //update_data.waveform_mode = MXCFB_WAVEFORM_MODE_INIT;
        update_data.waveform_mode = MXCFB_WAVEFORM_MODE_AUTO;
        update_data.update_marker = 1;
        update_data.flags = MXCFB_FLAG_ENABLE_INVERSION | MXCFB_FLAG_USE_AAD;

        if (ioctl(fb, MXCFB_SEND_UPDATE, &update_data) < 0) {
            return ERROR_MXCFB_UPDATE;
        }

        memset(&marker_data, 0, sizeof(struct mxcfb_update_marker_data));
        marker_data.update_marker = update_data.update_marker;
        if (ioctl(fb, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker_data) < 0) {
            return ERROR_MXCFB_UPDATE;
        }
    }

    update_data.waveform_mode = MXCFB_WAVEFORM_MODE_AUTO;
    update_data.update_marker = 2;
    update_data.flags = MXCFB_FLAG_USE_AAD;

    if (ioctl(fb, MXCFB_SEND_UPDATE, &update_data) < 0) {
        return ERROR_MXCFB_UPDATE;
    }

    memset(&marker_data, 0, sizeof(struct mxcfb_update_marker_data));
    marker_data.update_marker = update_data.update_marker;
    if (ioctl(fb, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker_data) < 0) {
        return ERROR_MXCFB_UPDATE;
    }

    return NO_ERROR;
}

int refresh_ebc(int fb, int ebc) {
    struct ebc_buf_info frame[2];
    if (ioctl(ebc, EBCIO_GET_EBC_BUFFER, &frame[0]) < 0) {
        return ERROR_EBC_GET_BUF;
    }

    int buf_sz = frame[0].vir_width * frame[0].vir_height * 2;
    void *buf = mmap(0, buf_sz, PROT_READ | PROT_WRITE, MAP_SHARED, ebc, 0);

    if (buf == MAP_FAILED) {
        return ERROR_MMAP;
    }

    int ret = NO_ERROR;
    frame[0].epd_mode = EBC_EPD_AUTO;
    memset(buf + frame[0].offset, 0xFF, frame[0].vir_width * frame[0].vir_height / 2);

    if (ioctl(ebc, EBCIO_SET_EBC_SEND_BUFFER, &frame[0]) < 0) {
        ret = ERROR_EBC_SET_BUF;
        goto exit;
    }

    if (ioctl(ebc, EBCIO_GET_EBC_BUFFER, &frame[1]) < 0) {
        return ERROR_EBC_GET_BUF;
    }

    frame[1].epd_mode = EBC_EPD_AUTO;

    if (ioctl(ebc, EBCIO_SET_EBC_SEND_BUFFER, &frame[1]) < 0) {
        ret = ERROR_EBC_SET_BUF;
        goto exit;
    }

exit:
    munmap(buf, buf_sz);
    return ret;
}

int main(int argc, char **argv) {
    uint8_t ret;
    int fb = -1;
    int ebc = -1;

    if (argc < 2) {
        ret = ERROR_INV_ARGS;
        goto exit;
    }

    int sec = atoi(argv[1]);
    usleep(sec * 1000);

    int mode = 0;
    if (argc >= 3) {
        mode = atoi(argv[2]);
    }

    fb = open_framebuffer();
    if (fb < 0) {
        ret = ERROR_OPEN_FB;
        goto exit;
    }

    if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix_info) < 0) {
        ret = ERROR_GET_FINFO;
        goto exit;
    }

    if (IS_MXCFB(fb_fix_info.id)) {
        ret = refresh_mxcfb(fb, mode);
        goto exit;
    }

    ebc = open_ebc();
    if (ebc < 0) {
        ret = ERROR_OPEN_EBC;
        goto exit;
    }
    ret = refresh_ebc(fb, ebc);

exit:
    if (fb >= 0) {
        close_framebuffer(fb);
    }
    if (ebc >= 0) {
        close_ebc(ebc);
    }
    return ret;
}
