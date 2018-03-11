#ifndef __EBC_H__
#define __EBC_H__

enum {
    EBC_EPD_AUTO = 0,
    EBC_EPD_FULL = 1,
    EBC_EPD_PART = 3,
};

#define EBCIO_GET_EBC_BUFFER        0x7000
#define EBCIO_SET_EBC_SEND_BUFFER   0x7001
#define EBCIO_GET_EBC_DRIVER_SN     0x7002
#define EBCIO_GET_EBC_BUFFER_INFO   0x7003

struct ebc_buf_info {
	int offset;
	int epd_mode;
	int height;
	int width;
	int vir_height;
	int vir_width;
	int fb_width;
	int fb_height;
	int color_panel;
	int win_x1;
	int win_y1;
	int win_x2;
	int win_y2;
	int rotate;
};

struct ebc_sn_info {
	uint32_t key;
	uint32_t sn_len;
	char cip_sn[256];
};

#endif
