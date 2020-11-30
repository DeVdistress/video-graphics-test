/*
* Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef DISP_OBJ_H
#define DISP_OBJ_H

#include "qpa_ctl.h"

//Max HW planes on AM57x DSS is 4. One owned by primary and 3 remianed for overlays
#define MAX_OVERLAY_PLANE 3

class DispObj{
public: 
    DispObj(QpaCtlObj *qpa_ctl, void *screen, uint32_t x, 
        uint32_t y, uint32_t w, uint32_t h, uint32_t num_planes);
    ~DispObj();
    int set_properties(uint32_t src_w, uint32_t src_h, 
        uint32_t fb_id, uint32_t plane_idx);
    int export_buf2_qpa(uint32_t w, uint32_t h, uint32_t pix_fmt, 
        uint32_t *pitches, uint32_t *offset, uint32_t fd, uint32_t *fb_id);
    int queue_buf(uint32_t plane_idx, uint32_t fb_id);
    int dequeue_bufs(int wait_time);
    int start_disp();
private:
    QpaCtlObj *m_qpa_ctl;
    void *m_screen;
    uint32_t m_crtc_x;
    uint32_t m_crtc_y;
    uint32_t m_crtc_w;
    uint32_t m_crtc_h;
    uint32_t m_num_planes;
	bool m_page_flipped;
    uint32_t m_plane_id[MAX_OVERLAY_PLANE];
};

#endif // DISP_OBJ_H
