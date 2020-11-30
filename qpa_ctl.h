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

#ifndef QPA_CTL_H
#define QPA_CTL_H

#include <QGuiApplication>

typedef int (*fnptr_export_buf)(void *screen, uint32_t width, 
                                uint32_t height, uint32_t pixel_format,	uint32_t *strides, 
                                uint32_t *offsets, uint32_t buf_fd, uint32_t *buf_id);
typedef int (*fnptr_distroy_buf)(void *screen, uint32_t buf_id);

typedef	int (*fnptr_create_pln)(void *screen);
typedef	int (*fnptr_distroy_pln)(void *screen, uint32_t idx);

typedef	int (*fnptr_set_pln_prop)(void *screen, uint8_t planeType,
                                  uint32_t idx, uint8_t num_prop, const char **prop_name, uint32_t *prop_val);
typedef	int (*fnptr_get_pln_prop)(void *screen, uint8_t planeType,
                                  uint32_t idx, const char *prop_name);

typedef int (*fnptr_queue_pln)(void *screen, uint32_t idx, uint32_t fb_id);
typedef int (*fnptr_start_disp_pln)(void *screen);
typedef	int (*fnptr_dequeue_pln)(void *screen, int block_time);
typedef	int (*fnptr_set_user_callbk)(void *screen, void (*func)(void *), void *data);

class QpaCtlObj{
public:
    QpaCtlObj(QGuiApplication *a);
    fnptr_export_buf    m_export_buffer;
    fnptr_distroy_buf   m_distroy_buffer;
    fnptr_create_pln    m_create_plane;
    fnptr_distroy_pln   m_distroy_plane;
    fnptr_set_pln_prop  m_set_plane_properties;
    fnptr_get_pln_prop  m_get_plane_property;
    fnptr_queue_pln		m_queue_buffer;
    fnptr_dequeue_pln   m_dequeue_buffer;
    fnptr_start_disp_pln m_start_disp_plane;
    fnptr_set_user_callbk m_set_user_call_back;
};

#endif // QPA_CTL_H
