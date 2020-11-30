#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <linux/videodev2.h>

#define NBUF (3)
#define CAP_WIDTH 704
#define CAP_HEIGHT 280



/*
* V4L2 capture device structure declaration
*/
class V4l2Obj {
public: 
	V4l2Obj();
	V4l2Obj(const char * dev_name, int w, int h, int num_buf);
   ~V4l2Obj();
    int request_buf(int *fd);
    int queue_buf(int fd);
    int dequeue_buf();
private:
	int m_fd;
	enum v4l2_memory m_memory_mode;
	unsigned int num_buffers;
	int m_width;
	int m_height;
	char m_dev_name[256];
	struct v4l2_buffer *m_v4l2buf;
	struct v4l2_format m_fmt;

	void default_parameters();
	void device_init();
};

/*
* Initialize the app resources with default parameters
*/
void V4l2Obj::default_parameters(void)
{
	/* Main camera */
	m_memory_mode = V4L2_MEMORY_DMABUF;
	m_num_buffers = NBUF;
	strcpy(m_dev_name,"/dev/video1");
	m_buffers = NULL;
    m_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	m_width = CAP_WIDTH;
	m_height = CAP_HEIGHT;

	return;
}

void V4l2Obj::device_init()
{
		/* Open the capture device */
	m_fd = open(m_dev_name, O_RDWR);
	if (m_fd <= 0) {
		printf("Cannot open %s device\n", m_dev_name);
		return -1;
	}

	MSG("\n%s: Opened Channel\n", m_dev_name);

	/* Check if the device is capable of streaming */
	if (ioctl(m_fd, VIDIOC_QUERYCAP, &capability) < 0) {
		perror("VIDIOC_QUERYCAP");
		goto ERROR;
	}

	if (capability.capabilities & V4L2_CAP_STREAMING)
		MSG("%s: Capable of streaming\n", m_dev_name);
	else {
		ERROR("%s: Not capable of streaming\n", m_dev_name);
		goto ERROR;
	}

	streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(m_fd, VIDIOC_G_PARM, &streamparam) < 0){
		perror("VIDIOC_G_PARM");
		goto ERROR;
	}

	m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(m_fd, VIDIOC_G_FMT, &m_fmt);
	if (ret < 0) {
		ERROR("VIDIOC_G_FMT failed: %s (%d)", strerror(errno), ret);
		goto ERROR;
	}

	m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_fmt.fmt.pix.width = m_width;
	m_fmt.fmt.pix.height = m_height;

	ret = ioctl(m_fd, VIDIOC_S_FMT, &m_fmt);
	if (ret < 0) {
		perror("VIDIOC_S_FMT");
		goto ERROR;
	}

	MSG("%s: Init done successfully\n", dev_name);

ERROR:
	close(m_fd);
}

V4l2Obj::V4l2Obj()
{
	int ret;
	struct v4l2_capability capability;
	struct v4l2_streamparm streamparam;

	default_parameters();
	v4l2_device_init();
}

V4l2Obj::V4l2Obj(const char * dev_name, int w, int h, int num_buf, int pix_fmt)
{
	int ret;
	struct v4l2_capability capability;
	struct v4l2_streamparm streamparam;

	default_parameters();

    m_dev_name = dev_name;
	m_width = w;
	m_height = h;
	m_num_buffers = num_buf;
    m_fmt.fmt.pix.pixelformat = pix_fmt;

	v4l2_device_init();
}

V4l2Obj::~V4l2Obj(struct v4l2_device_info *device)
{

	free(m_v4l2buf);
	close(m_fd);

	return;
}

int V4l2Obj::request_buf(int *fd)
{
	struct v4l2_requestbuffers reqbuf;
	unsigned int i;
	int ret;

	if (m_v4l2buf) {
		// maybe eventually need to support this?
		ERROR("already reqbuf'd");
		return -1;
	}

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = m_memory_mode;
	reqbuf.count = m_num_buffers;

	ret = ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		ERROR("VIDIOC_REQBUFS failed: %s (%d)", strerror(errno), ret);
		return ret;
	}

	if ((reqbuf.count != device->num_buffers) ||
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
int V4l2Obj::queue_buf(int fd)
{
	struct v4l2_buffer *v4l2buf = NULL;
	int  ret;
	unsigned char i;

	if(buf->nbo != 1){
		ERROR("number of bufers not right\n");
		return -1;
	}

	for (i = 0; i < device->num_buffers; i++) {
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
int V4l2Obj::dequeue_buf()
{
	struct dmabuf_buffer *buf;
	struct v4l2_buffer v4l2buf;
	int ret;

	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_DMABUF;
	ret = ioctl(m_fd, VIDIOC_DQBUF, &v4l2buf);
	if (ret) {
		ERROR("VIDIOC_DQBUF failed: %s (%d)\n", strerror(errno), ret);
		return NULL;
	}

	m_v4l2buf[v4l2buf.index].timestamp = v4l2buf.timestamp;

	return v4l2buf.index;
}




/*
* Enable streaming for V4L2 capture device
*/
int V4l2Obj::stream_on()
{
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
int V4l2Obj::stream_off()
{
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
