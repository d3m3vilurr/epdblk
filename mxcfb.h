/*
 * Copyright 2004-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

/*
 * @file arch-mxc/   mxcfb.h
 *
 * @brief Global header file for the MXC Frame buffer
 *
 * @ingroup Framebuffer
 */
#ifndef __MXCFB_H__
#define __MXCFB_H__

#define MXCFB_MAGIC0 0x5F63786D // mxc_
#define MXCFB_MAGIC1 0x63647065 // epdc
#define IS_MXCFB(id) \
    (*(uint32_t*)(id) == MXCFB_MAGIC0) && \
    (*(uint32_t*)((id) + 4) == MXCFB_MAGIC1)

enum {
    MXCFB_WAVEFORM_MODE_INIT        = 0x0,
    MXCFB_WAVEFORM_MODE_DU          = 0x1,
    MXCFB_WAVEFORM_MODE_GC16        = 0x2,
    MXCFB_WAVEFORM_MODE_AUTO        = 0x101,
};

enum {
    MXCFB_UPDATE_MODE_PARTIAL = 0x0,
    MXCFB_UPDATE_MODE_FULL    = 0x1,
};

enum {
    MXCFB_FLAG_ENABLE_INVERSION = 0x0001,
    MXCFB_FLAG_FORCE_MONOCHROME = 0x0002,
    MXCFB_FLAG_USE_AAD          = 0x1000,
};

enum {
    MXCFB_TEMP_USE_REMARKABLE_DRAW  = 0x0018,
    MXCFB_TEMP_USE_AMBIENT          = 0x1000,
    MXCFB_TEMP_USE_PAPYRUS          = 0x1001,
    MXCFB_TEMP_USE_MAX              = 0xFFFF,
};

enum {
    MXCFB_UPDATE_SCHEME_SNAPSHOT        = 0x0,
    MXCFB_UPDATE_SCHEME_QUEUE           = 0x1,
    MXCFB_UPDATE_SCHEME_QUEUE_AND_MERGE = 0x2,
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

struct mxcfb_update_marker_data {
    uint32_t update_marker;
    uint32_t collision_test;
};

#define MXCFB_SEND_UPDATE               _IOW('F', 0x2E, struct mxcfb_update_data)
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE  _IOWR('F', 0x2F, struct mxcfb_update_marker_data)
#define MXCFB_UPDATE_SCHEME             _IOW('F', 0x32, uint32_t)

#endif
