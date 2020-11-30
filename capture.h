#ifndef LOOP
#define LOOPBACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*fnptr_add_buf)(void *screen, uint32_t width, 
		uint32_t height, uint32_t pixel_format,	uint32_t *strides, 
		uint32_t *offsets, uint32_t buf_fd, uint32_t *buf_id);

typedef	int (*fnptr_crt_pln)(void *screen);

typedef	int (*fnptr_set_pln_prop)(void *screen, uint8_t planeType,
	uint32_t idx, uint8_t num_prop, const char **prop_name, uint32_t *prop_val);

typedef int (*fnptr_queue_overlay_pln)(void *screen, uint32_t idx, uint32_t fb_id);
typedef	int (*fnptr_disp_usr_pln)(void *screen);
typedef	int (*fnptr_free_usr_pln)(void *screen, int block_time);

struct control {
    unsigned int main_cam;
    unsigned int num_cams;
    unsigned int num_jpeg;
    unsigned int display_xres, display_yres;
    bool pip;
    bool jpeg;
    bool exit;
	void *screen;
	void *screen1;
	int drm_fd;
    fnptr_add_buf add_overlay_buffer;				// buffer for overlay callback
	fnptr_crt_pln create_overlay_plane; 			// create overlay plane
	fnptr_set_pln_prop set_plane_property;			// set property for plane callback
	fnptr_queue_overlay_pln queue_overlay_plane;
	fnptr_disp_usr_pln disp_user_plane;				// displaing of user plane
	fnptr_free_usr_pln free_overlay_plane;			// free of plane
};

extern struct control status;

int init_loopback(void);
void process_frame(void);
void end_streaming(void);
void exit_devices(void);
void drm_disable_pip(void);
void drm_enable_pip(void);
void set_plane_properties(void);

#ifdef __cplusplus
}
#endif

#endif // LOOPBACK_H
