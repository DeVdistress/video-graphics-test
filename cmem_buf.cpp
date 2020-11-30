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

/* This file defines function to allocate memory from CMEM pool */

#include <iostream>
#include <ti/cmem.h>
#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#include "cmem_buf.h"
#include "error.h"

#define CMEM_BLOCKID CMEM_CMABLOCKID

CMEM_AllocParams cmem_alloc_params = {
    CMEM_HEAP,      /* type */
    CMEM_CACHED,    /* flags */
    1               /* alignment */
};

bool g_is_cmem_init=false;
bool g_is_cmem_exit=false;


int BufObj::alloc_cmem_buffer(uint32_t size, uint32_t align, void **cmem_buf){
    cmem_alloc_params.alignment = align;

    *cmem_buf = CMEM_alloc2(CMEM_BLOCKID, size,
        &cmem_alloc_params);

    if(*cmem_buf == NULL){
        ERROR("CMEM allocation failed");
        return -1;
    }

    return CMEM_export_dmabuf(*cmem_buf);
}

void BufObj::free_cmem_buffer(void *cmem_buffer){
    CMEM_free(cmem_buffer, &cmem_alloc_params);
}

 /* If the user space need to access the CMEM buffer for CPU based processing, it can set
    the CMEM buffer cache settings using DMA_BUF IOCTLS. 
    Cahche_operation setting for invalidation is (DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ))
	Cache operation settting for buffer read write is (DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_READ
	This piece of code is not used in the application and hence not tested
*/
int BufObj::dma_buf_do_cache_operation(int dma_buf_fd, uint32_t cache_operation){
    int ret;
    struct dma_buf_sync sync;
    sync.flags = cache_operation;

    ret = ioctl(dma_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);

    return ret;
}

BufObj::BufObj(uint32_t w, uint32_t h, uint32_t bpp,uint32_t fourcc, 
               uint32_t align, uint32_t num_bufs){
    m_width = w;
    m_height = h;

    /* Vivante HAL needs 16 pixel alignment in width and 4 pixel alignment in
    * height and hence putting that restriction for now on all buffer allocation through CMEM.
    */
    m_stride = ((w + 15) & ~15) * bpp;
    m_fourcc = fourcc;
    m_num_bufs = num_bufs;

    //Avoid calling CMEM init with every BufObj object
    if(g_is_cmem_init == false){
        CMEM_init();
        g_is_cmem_init = true;
    }

    m_fd = (int *)malloc(num_bufs * sizeof(int));

    if(m_fd == NULL){
        ERROR("DispObj: m_fd array allocation failure\n");
    }

    m_fb_id = (uint32_t *)malloc(num_bufs * sizeof(int));
    if(m_fb_id == NULL){
        ERROR("DispObj: m_fb_id array allocation failure\n");
    }

    m_buf = (void **)malloc(num_bufs * sizeof(int));
    if(m_buf == NULL){
        ERROR("DispObj: m_buf array allocation failure\n");
    }

    for(uint32_t i = 0; i < num_bufs; i++){

        /* Vivante HAL needs 16 pixel alignment in width and 4 pixel alignment in
        * height and hence adjust the buffer size accrodingly.
        */
        uint32_t size = m_stride * ((m_height + 3) & ~3);

        m_fd[i]  = alloc_cmem_buffer(size, align, &m_buf[i]);

		if(m_buf[i] == NULL){
			ERROR("CMEM buffer allocation failed");
		}

        if(m_fd[i] < 0){
            free_cmem_buffer(m_buf[i]);
            ERROR("Cannot export CMEM buffer");
        }
    }
}


BufObj::~BufObj(){

    uint32_t i;

    for(i = 0; i < m_num_bufs; i++){
        //Free the buffer
        free_cmem_buffer(m_buf[i]);
    }

    free(m_fd);
    free(m_fb_id);
    free(m_buf);

    if(g_is_cmem_exit == false){
        CMEM_exit();
        g_is_cmem_exit = true;
    }
}
