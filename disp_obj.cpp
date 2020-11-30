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

/* This file defines the functions to deal with the DSS engine via QT eglfs_kms QPA 
*  It secures the hardware plane for overlay need, set one time the display proerpties 
*  of the overlay plane, also sets the primary plane properties for alphablending, 
*  exports user allocated video buffers to the DRM, queue and dequeue the video buffers
*  submitted to the QPA 
*/

#include <sys/types.h>
#include <stdint.h>
#include <xf86drmMode.h>
#include <unistd.h>
#include "disp_obj.h"
#include "error.h"
#include <sys/time.h>

static void page_flip_handler(void *data){
    bool *page_flipped = (bool *)data; 
	//MSG("received notification from QPA\n");
    *page_flipped = true;
}

DispObj::DispObj(QpaCtlObj *qpa_ctl, void *screen, uint32_t x, 
                 uint32_t y, uint32_t w, uint32_t h, uint32_t num_planes) {
    m_screen =  screen;
    m_crtc_x = x;
    m_crtc_y = y;
    m_crtc_w = w;
    m_crtc_h = h;
    m_num_planes = num_planes;
    m_page_flipped = false;

    m_qpa_ctl = qpa_ctl;

    //If display is not connected, don't create overlay planes
    if(screen != NULL){

        //Secure DSS hardware planes for overlay from QT QPA
        for(uint32_t i = 0; i < m_num_planes; i++){
            m_plane_id[i] = m_qpa_ctl->m_create_plane(m_screen);
        }
    
        /* Pass callback function pointer to the QT eglfs_kms QPA. The QPA will
         call this function to notify display completion of the overlay buffer */
        m_qpa_ctl->m_set_user_call_back(m_screen, &page_flip_handler, &m_page_flipped);
    }
}

DispObj::~DispObj(){
    for(uint32_t i = 0; i < m_num_planes; i++){
        m_qpa_ctl->m_distroy_plane(m_screen, m_plane_id[i]);
    }
}

/*
* Set up the DSS for scaling of video planes and blending it with graphics planes
*/
int DispObj::set_properties(uint32_t src_w, uint32_t src_h, 
    uint32_t fb_id, uint32_t plane_idx){

    char trans_key_mode = 2;
    char *propname[11];
    unsigned int propval[11];
    int i;

    for(i = 0; i < 11; i++)
    {
        propname[i]= (char *) malloc(sizeof(char)*128);
    }

    /*set primary/graphics plane property for alpha blending*/
    strcpy(propname[0],"trans-key-mode");
    strcpy(propname[1],"alpha_blender");
    strcpy(propname[2],"zorder");

    propval[0] = trans_key_mode;
    propval[1] = 1;
    propval[2] = 1;

    m_qpa_ctl->m_set_plane_properties(m_screen, DRM_PLANE_TYPE_PRIMARY, 0, 
        3, (const char**)propname, propval);

    /* Set overlay plane properties for input and output resolution (scaling) */
    /* CRTC properties below are the DSS processed/displayed output proerties */
    /* and SRC is for input */
    strcpy(propname[0], "CRTC_X"); //output offset X to be displayed from
    propval[0] = m_crtc_x;
    strcpy(propname[1], "CRTC_Y"); //output offset Y to be displayed from
    propval[1] = m_crtc_y;
    strcpy(propname[2], "CRTC_W"); //output width, set to screen width in this example
    propval[2] = m_crtc_w;
    strcpy(propname[3], "CRTC_H"); //output width, set to screen height in this example
    propval[3] = m_crtc_h;
    strcpy(propname[4], "SRC_X"); //input buffer offset X to be processed from
    propval[4] = 0;
    strcpy(propname[5], "SRC_Y"); //input buffer offset Y to be processed from
    propval[5] = 0;
    strcpy(propname[6], "SRC_W"); //input buffer width
    propval[6] = src_w << 16 ;
    strcpy(propname[7], "SRC_H"); //input buffer height
    propval[7] = src_h << 16;
    strcpy(propname[8], "zorder"); //order of the video plane when laying on top/below other planes
    propval[8] = 2;                // higher the value, higher in order the plane reside. Max value is 3
    strcpy(propname[9], "FB_ID");
    propval[9] = fb_id;            //frame buffer id
    strcpy(propname[10], "global_alpha");
    propval[10] = 255;             //Alpha value for blending

    /* Set the plane properties once. After that only set those properties that changes each time a frame is 
    displayed - like fb_id. */
    m_qpa_ctl->m_set_plane_properties(m_screen, DRM_PLANE_TYPE_OVERLAY, m_plane_id[plane_idx], 
        11, (const char**)propname, propval);

    for (i = 0; i < 11; i++){
        free(propname[i]);
    }

    return 0;
}

/* Export user allocated buffer to the QT QPA/DRM device */
int DispObj::export_buf2_qpa(uint32_t w, uint32_t h, uint32_t pix_fmt, 
        uint32_t *pitches, uint32_t *offset, uint32_t fd, uint32_t *fb_id){
    return m_qpa_ctl->m_export_buffer(m_screen, w, h, pix_fmt, pitches, offset, fd, fb_id);
}

/* Queue the video buffer to be overlayed to QPA */
int DispObj::queue_buf(uint32_t plane_idx, uint32_t fb_id){
    m_qpa_ctl->m_queue_buffer(m_screen, plane_idx, fb_id);
    return 0;
}

// Single wait for all the planes submitted to one display as they all get displayed together  
int DispObj::dequeue_bufs(int wait_time){
    struct timeval start, now;
    gettimeofday(&start, NULL);

    if(!wait_time){
        wait_time = 60; //by default, wait for 60 ms
    }
	//MSG("before m_page_flipped = %d", m_page_flipped);
	do{
		if (m_page_flipped == false){
			usleep(500);
		}
		else{
			m_page_flipped = false;
			return 0;
		}
		gettimeofday(&now, NULL);
	}while(((now.tv_usec - start.tv_usec)/1000) < wait_time);
	//MSG("after m_page_flipped = %d", m_page_flipped);
    MSG("Waited for %d msec. Overlay buffer wasn't freed\n", wait_time);
    return -1;
}

int DispObj::start_disp(){
    return(m_qpa_ctl->m_start_disp_plane(m_screen));
}
