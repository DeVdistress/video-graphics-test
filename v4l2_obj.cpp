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

/* This file defines the function to program V4L2 device */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include "v4l2_obj.h"
#include "error.h"

#define NBUF (3)
#define CAP_WIDTH 704
#define CAP_HEIGHT 280

/*
* Initialize the app resources with default parameters
*/
void V4l2Obj::default_parameters(void) {
    /* Main camera */
    m_num_buffers = NBUF;
    strcpy(m_dev_name,"/dev/video1");
    m_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    m_width = CAP_WIDTH;
    m_height = CAP_HEIGHT;
    m_v4l2buf = NULL;

    return;
}

void V4l2Obj::device_init(int pix_fmt){
    struct v4l2_capability capability;
    struct v4l2_streamparm streamparam;
    int ret;

    /* Open the capture device */
    m_fd = open(m_dev_name, O_RDWR);

    if (m_fd <= 0) {
        ERROR("Cannot open %s device\n\n", m_dev_name);
        return;
    }

    MSG("\n%s: Opened Channel\n", m_dev_name);

    /* Check if the device is capable of streaming */
    if (ioctl(m_fd, VIDIOC_QUERYCAP, &capability) < 0) {
        perror("VIDIOC_QUERYCAP");
        goto ERR;
    }

    if (capability.capabilities & V4L2_CAP_STREAMING)
        MSG("%s: Capable of streaming\n", m_dev_name);
    else {
        ERROR("%s: Not capable of streaming\n", m_dev_name);
        goto ERR;
    }

    streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(m_fd, VIDIOC_G_PARM, &streamparam) < 0){
        ERROR("VIDIOC_G_PARM");
        goto ERR;
    }

    m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(m_fd, VIDIOC_G_FMT, &m_fmt);
    if (ret < 0) {
        ERROR("VIDIOC_G_FMT failed: %s (%d)", strerror(errno), ret);
        goto ERR;
    }

    m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_fmt.fmt.pix.width = m_width;
    m_fmt.fmt.pix.height = m_height;
    m_fmt.fmt.pix.pixelformat = pix_fmt;

    ret = ioctl(m_fd, VIDIOC_S_FMT, &m_fmt);
    if (ret < 0) {
        ERROR("VIDIOC_S_FMT");
        goto ERR;
    }

    MSG("%s: Init done successfully\n", m_dev_name);
    return;
ERR:
    close(m_fd);
    return;
}

V4l2Obj::V4l2Obj(){
    default_parameters();
    device_init(V4L2_PIX_FMT_YUYV);
}

V4l2Obj::V4l2Obj(const char * dev_name, int w, int h, int pix_fmt, int num_buf){
    default_parameters();

    strcpy(m_dev_name,dev_name);
    m_width = w;
    m_height = h;
    m_num_buffers = num_buf;
    //m_fmt.fmt.pix.pixelformat = pix_fmt;

    device_init(pix_fmt);
}

V4l2Obj::~V4l2Obj(){
    free(m_v4l2buf);
    close(m_fd);

    return;
}

/* In this example appliaction, user space allocates the buffers and 
 * provides the buffer fd to be exported to the V4L2 driver 
*/
int V4l2Obj::request_buf(int *fd){
    struct v4l2_requestbuffers reqbuf;
    uint32_t i;
    int ret;

    if (m_v4l2buf) {
        // maybe eventually need to support this?
        ERROR("already reqbuf'd");
        return -1;
    }

    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_DMABUF;
    reqbuf.count = m_num_buffers;

    ret = ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret < 0) {
        ERROR("VIDIOC_REQBUFS failed: %s (%d)", strerror(errno), ret);
        return ret;
    }

    if ((reqbuf.count != m_num_buffers) ||
        (reqbuf.type != V4L2_BUF_TYPE_VIDEO_CAPTURE) ||
        (reqbuf.memory != V4L2_MEMORY_DMABUF)) {
            ERROR("unsupported..");
            return -1;
    }

    m_num_buffers = reqbuf.count;
    m_v4l2buf = (struct v4l2_buffer *) calloc(m_num_buffers, \
        sizeof(struct v4l2_buffer));
    if (!m_v4l2buf) {
        ERROR("allocation failed");
        return -1;
    }

    for (i = 0; i < m_num_buffers; i++) {
        m_v4l2buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_v4l2buf[i].memory = V4L2_MEMORY_DMABUF;
        m_v4l2buf[i].index = i;

        ret = ioctl(m_fd, VIDIOC_QUERYBUF, &m_v4l2buf[i]);
        m_v4l2buf[i].m.fd = fd[i];

        if (ret) {
            ERROR("VIDIOC_QUERYBUF failed: %s (%d)", strerror(errno), ret);
            return ret;
        }
    }

    return 0;
}

/*
* Queue V4L2 buffer
*/
int V4l2Obj::queue_buf(int fd){
    struct v4l2_buffer *v4l2buf = NULL;
    int  ret;
    unsigned char i;


    for (i = 0; i < m_num_buffers; i++) {
        if (m_v4l2buf[i].m.fd == fd) {
            v4l2buf = &m_v4l2buf[i];
        }
    }

    if (!v4l2buf) {
        ERROR("invalid buffer");
        return -1;
    }
    ret = ioctl(m_fd, VIDIOC_QBUF, v4l2buf);
    if (ret) {
        ERROR("VIDIOC_QBUF failed: %s (%d)", strerror(errno), ret);
    }

    return ret;
}

/*
* DeQueue V4L2 buffer
*/
int V4l2Obj::dequeue_buf(){
    struct v4l2_buffer v4l2buf;
    int ret;

    v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2buf.memory = V4L2_MEMORY_DMABUF;
    ret = ioctl(m_fd, VIDIOC_DQBUF, &v4l2buf);
    if (ret) {
        ERROR("VIDIOC_DQBUF failed: %s (%d)\n", strerror(errno), ret);
        return -1;
    }

    m_v4l2buf[v4l2buf.index].timestamp = v4l2buf.timestamp;

    return v4l2buf.index;
}

/*
* Enable streaming for V4L2 capture device
*/
int V4l2Obj::stream_on(){
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = 0;

    ret = ioctl(m_fd, VIDIOC_STREAMON, &type);

    if (ret) {
        ERROR("VIDIOC_STREAMON failed: %s (%d)", strerror(errno), ret);
    }

    return ret;
}

/*
* Disable streaming for V4L2 capture device
*/
int V4l2Obj::stream_off(){
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = -1;

    if (m_fd <= 0) {
        return ret;
    }

    ret = ioctl(m_fd, VIDIOC_STREAMOFF, &type);

    if (ret) {
        ERROR("VIDIOC_STREAMOFF failed: %s (%d)", strerror(errno), ret);
    }

    return ret;
}
