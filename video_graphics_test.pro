TEMPLATE = app

CONFIG += $$CONFIG_RD

QT += quick core gui-private
LIBS += -lticmem 
LIBS += -lGAL 
LIBS += -lstdc++

INCLUDEPATH += $(SDK_PATH_TARGET)/usr/include/libdrm \
	       $(SDK_PATH_TARGET)/usr/include/omap \
	       $(SDK_PATH_TARGET)/usr/include/HAL2D
	       
SOURCES = main.cpp cmem_buf.cpp v4l2_obj.cpp qpa_ctl.cpp disp_obj.cpp video_graphics_test.cpp gc320.cpp
HEADERS  +=  cmem_buf.h v4l2_obj.h qpa_ctl.h disp_obj.h video_graphics_test.h gc320.h

RESOURCES = video_graphics_test.qrc
OTHER_FILES = screen.qml
INSTALLS += target