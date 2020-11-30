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

#ifndef CMEM_BUF_H
#define CMEM_BUF_H

class BufObj {
public:
    BufObj(uint32_t w, uint32_t h, uint32_t bpp, uint32_t fourcc, uint32_t align, uint32_t num_bufs);
    ~BufObj();

    //If applications wants to do cache operations on buffer
    int dma_buf_do_cache_operation(int dma_buf_fd, uint32_t cache_operation);

    uint32_t m_fourcc;
    uint32_t m_width;
    uint32_t m_height; 
    uint32_t m_stride;

    int *m_fd;		
    uint32_t *m_fb_id;
    void **m_buf;

private:
    int alloc_cmem_buffer(unsigned int size, unsigned int align, void **cmem_buf);
    void free_cmem_buffer(void *cmem_buffer);
    uint32_t m_num_bufs;
};

#endif //CMEM_BUF_H
