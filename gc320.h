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

#ifndef GC320_H
#define GC320_H

#include <gc_hal.h>
#include <gc_hal_raster.h>

#define MAX_MINORFEATURE_NUMBER 10
#define MIRROR


struct surf_context{
    gcoSURF surf;
    uint32_t phys_address[4];
};

/// Uncomment to enable more debugging prints
#define DEBUG_GC320

class Gc320Obj
{

public:

    /// Constructor
    Gc320Obj(uint32_t num_src, uint32_t num_src_surf, uint32_t num_dst_surf, BufObj **src_bo, BufObj *dst_bo);
    ~Gc320Obj();

    /**
    * gc320 configure_src_surface()     * Purpose: Construct source surface
    *
    * @return Return bool success or failure
    */
    int configure_surface(surf_context *surf_ctx, uint32_t num_ctx, BufObj *bo, gceSURF_FORMAT surf_format);

    /**
    * gc320 render_90degree_rotation()
    * Purpose: Program GC320 to rotate the  surface 90 degree
    *
    * @param index to source surface
    * @param index to desitination surface
    *
    * @return bool true/success or false/failure
    */
    bool rotation_90deg(uint32_t a, uint32_t b);

    /**
    * gc320 composition()
    * Purpose: Program GC320 to stitch two images together
    * One image will be rotated 90 degrees and the second image will be rotated 0 degrees
    *
    * @param index to source surface
    * @param index to desitination surface
    *
    * @return bool true/success or false/failure
    */
    bool composition(uint32_t src_surf_indx, uint32_t dst_surf_indx);


private:

    /**
    * gc320 chip_check()
    * Purpose: Checkout the GC320 Hardware and print info on it
    *
    *
    * @return 0 on success < 0 on failure
    */
    int chip_check();


    /**
    * gc320 initGC30Hardware()
    * Purpose: Probe and initialize the GC320 Chip
    *
    * @return 0 on Success and -1 on Failure
    */
    int init_gc320();

    // Runtime parameters
    gcoOS           m_os;
    gcoHAL          m_hal;
    gco2D           m_engine2d;

    /* Dest surface. (1 Dest. Surface) */
    gceSURF_FORMAT m_dst_format;
    uint32_t	   m_dst_width;
    uint32_t	   m_dst_height;
    uint32_t	   m_dst_stride;
    surf_context  *m_dst_surf_ctxt;
    uint32_t       m_num_dst_ctx;

    /* Source surface. (1 or more Source Surface) */
    gceSURF_FORMAT m_src_format;
    uint32_t	   *m_src_width;
    uint32_t	   *m_src_height;
    uint32_t	   *m_src_stride;
    surf_context  **m_src_surf_ctxt;
    uint32_t       m_num_src_ctx;
    uint32_t	   m_num_src; // Defines number of source surface
};

#endif // GC320_H
