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

#include <QGuiApplication>
#include <QScreen>
#include <QThread>
#include <QQueue>
#include <unistd.h>
#include "cmem_buf.h"
#include "v4l2_obj.h"
#include "qpa_ctl.h"
#include "disp_obj.h"
#include "error.h"
#include "video_graphics_test.h"
#include "gc320.h"

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | \
    ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | \
    ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])

#define DISP_QUEUE_DEPTH 2
#define NUM_CAP_BUFS      (DISP_QUEUE_DEPTH+3)
#define NUM_GC320_BUFS    (DISP_QUEUE_DEPTH+1)
#define CAP_W 704
#define CAP_H 280
#define NUM_SRC 2  // Defines how many input sources the user wants to use
#define DEQUEUE_WAIT_TIME 100 //worst case wait for 30 ms before flagging error

uint32_t gc320_buf_idx[NUM_GC320_BUFS];

void run_video_graphics_test(BufObj *bo_cap, BufObj *bo_gc320, V4l2Obj *cap,
							 Gc320Obj *gc320_proc, DispObj *disp1, DispObj *disp2){
    bool dual_display = (g_qt_app->screens().count() > 1) ? true : false;
    int gc320_buf_idx = 0;
    int frm_cnt = 0, disp_qdepth = 0;
    struct timeval start, now;
	QQueue<uint32_t> cap_buf_idx;
    gettimeofday(&start, NULL);
    
    while(1){

        //Get the capture video
        int cap_buf_index = cap->dequeue_buf();

	//Call rotation_90deg for just rotating the image (By default not called)
        //Call GC320 for stitching two captured video together
        if(gc320_proc->composition(cap_buf_index, gc320_buf_idx) == false){
		ERROR("error running gc320 example\n");
	}

        //Queue the GC320 processed video to scale, overlay and alpha blend with graphics and display on DISPLAY1
        //There can be multiple (on AM57x, upto 3 planes) video planes that can be submitted to same display
        disp1->queue_buf(0, bo_gc320->m_fb_id[gc320_buf_idx]);

        //Queue the captured video to scale, overlay and alpha blend with graphics and display on DISPLAY2
        if(dual_display)
            disp2->queue_buf(0, bo_cap->m_fb_id[cap_buf_index]);

        disp_qdepth++;

        cap_buf_idx.enqueue(cap_buf_index);

        /* Start displaying once the QT QPA display queue for user plane is full. Queue depth depends 
        upon the difference between capture and display processing rate. Display LCD panel/HDMI monitor
        on IDK and GP EVM can handle 60 fps. The camera on IDK can only capture 640x480 at 43 fps */
        if(disp_qdepth == DISP_QUEUE_DEPTH){
            disp1->start_disp();

            if(dual_display)
                disp2->start_disp();
        }

        if(disp_qdepth >= DISP_QUEUE_DEPTH){

            //Dequeue video plane buffer submitted on DISPLAY1
            disp1->dequeue_bufs(DEQUEUE_WAIT_TIME);

            //Dequeue video plane buffer submitted on DISPLAY2
            /* Since there are two seperate monitors running asynchronously and have
            there own display rate, it's not good idea to wait for the display completion
            for second monitor on same thread. Ideally there should be two independent thread 
            driving two monitors. For simplification of the app, we are doing in one single thread.
            Not waiting here for the display completion of second screeen is not having any impact 
            as GC320 processing and submission of it to display queue is happening sequentially.
            */
            //if(dual_display)
            //disp2->dequeue_bufs(DEQUEUE_WAIT_TIME);

            //Queue the released buffer to capture pool
            if (cap->queue_buf(bo_cap->m_fd[cap_buf_idx.dequeue()]) < 0) {
                exit(-1);
            }
        }

        gc320_buf_idx = (gc320_buf_idx+1) % NUM_GC320_BUFS;

        if(frm_cnt++ == 30){
            frm_cnt = 0;
            gettimeofday(&now, NULL);
            /* Performance will be govern by the slowest engine in the pipeline. In this 
            example, Ov2659 sensor streams VGA capture at 43 fps on IDK EVM and is the slowest in the pipeline.
            On GP EVM, MT9T111 will capture VGA @ 30 fps. 
            */
            MSG("time taken to process 30 frames in ms = %d\n", 
                (int) (((now.tv_sec - start.tv_sec)*1000) + (now.tv_usec - start.tv_usec)/1000));
            start = now;
        }
    }
}
void VideoGraphicsThread::run(){

    uint32_t cap_w = CAP_W;
    uint32_t cap_h = CAP_H;
    uint32_t num_src = NUM_SRC;// Set the number of source to 1 if running the rotation_90deg test (Default=2 sources) 

    // Going to create the GC320 output buffer 
    // Swap the width and height if running the rotation_90deg test (Not required for the composition test)
    uint32_t gc320_out_w = cap_w;
    uint32_t gc320_out_h = cap_h*2;

    // Set Display width/height
    uint32_t screen_w = cap_w;
    uint32_t screen_h = cap_h*2;

    bool dual_display = (g_qt_app->screens().count() > 1) ? true : false;

    //Create buffer objects for camera capture
    BufObj bo_cap(cap_w, cap_h, 2, FOURCC_STR("YUYV"), 1, NUM_CAP_BUFS);

    //Create VIP capture object
    //Open camera node on VIP1 and configure it to capture video of resoultion cap_w X cap_h in YUYv format
    V4l2Obj vip_cap("/dev/video1", cap_w, cap_h, FOURCC_STR("YUYV"), NUM_CAP_BUFS);

    // Request, configure the V4l2 buffers
    //Export the application allocated buffers (from CMEM driver) to VIP capture drivers 
    if (vip_cap.request_buf(bo_cap.m_fd) < 0) {
        ERROR("V4l2 capture buffers already allocated\n");
        exit(-1);
    }

    /*Create buffer objects for GC320 output
     * Vivante needs address to be 0x80 bytes aligned
     * Vivante HAL needs 16 pixel alignment in width and 4 pixel alignment in
     * height.
     */
    BufObj bo_gc320_out(gc320_out_w, gc320_out_h, 2, FOURCC_STR("YUYV"), 0x80, NUM_GC320_BUFS);

    //Create GC320 object
    //Initialize GC320 core, and it's input and output surfaces
    //
    //Note for the second input I am passing the same buffer object as the first input
    BufObj *gc320_in[num_src];
    for (uint32_t i = 0; i < num_src; i++){
	    gc320_in[i] = &bo_cap;// this can take buffer objects from different sources
    }

    Gc320Obj gc320_proc(num_src, NUM_CAP_BUFS, NUM_GC320_BUFS, gc320_in, &bo_gc320_out);

    //Create QT QPA object to get handles of various APIs from QT QPA to configure DSS hardware planes for video scaling and overlays
    QpaCtlObj qpa_ctl(g_qt_app);

    //Create Display1 Object
    //Pass on handles from QT QPA, create video overlay planes for DISPLAY1 and set the video plane display resolution  
    DispObj disp1(&qpa_ctl,  (void *)g_qt_app->screens().at(0), 0, 0, 
    		gc320_out_w, gc320_out_h, 1);
        //DeVdistress g_qt_app->screens().at(0)->geometry().width(), g_qt_app->screens().at(0)->geometry().height(), 1);

    //Create Display2 Object
    //Pass on handles from QT QPA, create video overlay planes for DISPLAY2 and set the video plane display resolution 
    uint32_t disp_w = 0;
    uint32_t disp_h = 0;
    void *screen = NULL;
    if(dual_display){
        screen = (void *)g_qt_app->screens().at(1);
        disp_w = g_qt_app->screens().at(1)->geometry().width();
        disp_h = g_qt_app->screens().at(1)->geometry().height();
    }
	
	DispObj disp2(&qpa_ctl, screen, 0, 0, disp_w, disp_h, 1);

	uint32_t offset = 0;
    for (uint32_t i = 0; i < NUM_GC320_BUFS; i++) {
	/* Export the video overlay buffers to DISPLAY1
	GC320 processed (90 degree rotated) video will be displayed on DISPLAY1
	Since the video is 90 degree rotated to original content, buffer width and height are swapped
	NO swapping required for the composition test
	*/
        disp1.export_buf2_qpa(screen_w, screen_h, FOURCC_STR("YUYV"), &bo_gc320_out.m_stride, &offset,  bo_gc320_out.m_fd[i], 
            &bo_gc320_out.m_fb_id[i]);
    }

    for (uint32_t i = 0; i < NUM_CAP_BUFS; i++) {
        //Queue capture buffers 
        vip_cap.queue_buf(bo_cap.m_fd[i]);

        //Export the video overlay buffers to DISPLAY2
        //Camera captured buffers will be displayed on DISPLAY2
        if(dual_display)
            disp2.export_buf2_qpa(screen_w, screen_h, FOURCC_STR("YUYV"), &bo_cap.m_stride, &offset,  bo_cap.m_fd[i], 
            &bo_cap.m_fb_id[i]);
    }

    //Set the DSS hardware video plane properties for Display1
    //Since we are rotating the capture buffer by 90 degree and displaying it, the width and height are swapped
    //NO swapping required for the composition test
    disp1.set_properties(screen_w, screen_h, bo_gc320_out.m_fb_id[0], 0);

    //Set the DSS hardware video plane properties for Display2
    if(dual_display)
        disp2.set_properties(screen_w, screen_h, bo_cap.m_fb_id[0], 0);

    //Start camera streaming
    if (vip_cap.stream_on() < 0) exit(-1);

    run_video_graphics_test(&bo_cap, &bo_gc320_out, &vip_cap, &gc320_proc, &disp1, &disp2);
}
