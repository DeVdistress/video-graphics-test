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
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QScreen>
#include <QDebug>
#include "video_graphics_test.h"

QGuiApplication *g_qt_app;

/*
This application demonstrates the advance processing capabilities of DSS IP when elfs QPA 
is used to run the QT application. eglfs QPA is used when there is a need for hardware
accelerated randering of the graphics content drawn by QT. eglfs QPA takes control of the 
display device and hence application relies on QT QPA to do the advance processing abilities of 
DSS IP like scaling, alpha blending and overlaying. Along with DSS advance capabilities using QT 
QPA for hardware accelerated rendering of the graphcis content on SGX, this application also 
demonstrates video rotation feature using GC320 IP, video capture using VIP interface and 
displaying these content on two screens. The data flow is as follows
             ---->GC320 stitch/rotate 90 degree --> QT QPA (draws the graphics content from qml file, scale, alpha blend and overlay (on DSS) the video with SGX output)
			|
VIP Capture -
            |
             ----> QT QPA (draws the graphics content from qml file, scale, alpha blend and overlay (on DSS) the video with SGX output)


*/
static QQuickView *addView(QScreen *screen, int screenIdx)
{
    qDebug("Creating new QQuickView for screen %p", screen);
    QQuickView *v = new QQuickView;

    v->setScreen(screen);
    v->setResizeMode(QQuickView::SizeViewToRootObject);//SizeRootObjectToView); //
    v->rootContext()->setContextProperty("screenIdx", screenIdx);
    v->rootContext()->setContextProperty("screenGeom", screen->geometry());
    v->rootContext()->setContextProperty("screenAvailGeom", screen->availableGeometry());
    v->rootContext()->setContextProperty("screenVirtGeom", screen->virtualGeometry());
    v->rootContext()->setContextProperty("screenAvailVirtGeom", screen->availableVirtualGeometry());
    v->rootContext()->setContextProperty("screenPhysSizeMm", screen->physicalSize());
    v->rootContext()->setContextProperty("screenRefresh", screen->refreshRate());

    if (screenIdx == 1){
        v->setSource(QUrl("qrc:/clocks.qml"));
    }
    else{
		v->setSource(QUrl("qrc:/maroon.qml"));
    }

    QObject::connect(v->engine(), &QQmlEngine::quit, qGuiApp, &QCoreApplication::quit);

    return v;
}

int main(int argc, char **argv)
{
	//Enable them when need to debug the QPA
#if(0)
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=true");
    qputenv("QSG_INFO", "1");
#endif

    QGuiApplication app(argc, argv);
    VideoGraphicsThread* video_graphics_test;

    QList<QScreen *> screens = app.screens();
    qDebug("Application sees %d screens", screens.count());
    qDebug() << screens;

#if(0)
    QVector<QQuickView *> views;
	//Set the QML based graphics content to be rendered for each screen
    for (int i = 0; i < screens.count(); ++i) {
        QQuickView *v = addView(screens[i], i);
        views.append(v);
        v->showFullScreen();
    }
#endif
    g_qt_app=&app;

	//Create and start the thread to capture, rotate and display the content 
    video_graphics_test = new VideoGraphicsThread;
    video_graphics_test->start();

    int r = app.exec();

#if(0)
    qDeleteAll(views);
#endif
    return r;
}

