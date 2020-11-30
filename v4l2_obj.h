#ifndef V4L2_OBJ_H
#define V4L2_OBJ_H

#include <linux/videodev2.h>
/*
* V4L2 Object class declaration
*/
class V4l2Obj {
public: 
	V4l2Obj();
	V4l2Obj(const char * dev_name, int w, int h, int pix_fmt, int num_buf);
   ~V4l2Obj();
    
   int request_buf(int *fd);
   int queue_buf(int fd);
   int dequeue_buf();
   int stream_off();
   int stream_on();

private:
	int m_fd;
	uint32_t m_num_buffers;
	uint32_t m_width;
	uint32_t m_height;
	char m_dev_name[256];
	struct v4l2_buffer *m_v4l2buf;
	struct v4l2_format m_fmt;
	void default_parameters();
	void device_init(int pix_fmt);
};

#endif // V4L2_OBJ_H
