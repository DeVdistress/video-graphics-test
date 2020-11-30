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

#include <qpa/qplatformnativeinterface.h>
#include "qpa_ctl.h"
#include "error.h"

/* eglfs_kms QT QPA provides API to user space so that it can submit the 
* video planes to be scaled, color converted and overlayed on top of graphics 
* planes and displayed to the panel/monitor. All eglfs_kms APIs function pointers 
* are stored in QpaCtlObj once and then later used by DispObJ to intercat with QT QPA
*/

QpaCtlObj::QpaCtlObj(QGuiApplication *a)
{
    QPlatformNativeInterface *platInf = a->platformNativeInterface();

    m_export_buffer =  (fnptr_export_buf)platInf->nativeResourceForIntegration("export_buffer");
    m_distroy_buffer =  (fnptr_distroy_buf)platInf->nativeResourceForIntegration("distroy_buffer");
    m_create_plane  = \
        (fnptr_create_pln)platInf->nativeResourceForIntegration("create_plane");
    m_distroy_plane  = \
        (fnptr_distroy_pln)platInf->nativeResourceForIntegration("distroy_plane");
    m_set_plane_properties = \
        (fnptr_set_pln_prop)platInf->nativeResourceForIntegration("set_plane_properties");
    m_get_plane_property = \
        (fnptr_get_pln_prop)platInf->nativeResourceForIntegration("get_plane_property");
    m_queue_buffer =  (fnptr_queue_pln)platInf->nativeResourceForIntegration("queue_plane");
    m_start_disp_plane =  (fnptr_start_disp_pln)platInf->nativeResourceForIntegration("start_disp_plane");
    m_set_user_call_back =  (fnptr_set_user_callbk)platInf->nativeResourceForIntegration("user_call_back_handle");
}

