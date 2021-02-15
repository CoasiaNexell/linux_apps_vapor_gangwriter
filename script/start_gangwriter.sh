#!/bin/sh

# export QT Settings
export QT_LOGGING_RULES='qt.qpa.*=false'
export QT_PLUGIN_PATH=/usr/lib/qt5/plugins
export QT_QPA_EGLFS_DEBUG=0
export QT_QPA_EGLFS_HIDECURSOR=0
export QT_QPA_EGLFS_KMS_CONFIG=/etc/qboot/eglfs_config.json
export QT_QPA_EGLFS_NO_LIBINPUT=1
export QT_QPA_EGLFS_SWAPINTERVAL=0
export QT_QPA_EGLFS_TSLIB=1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=tslib:/dev/input/touchscreen0:invertx:rotate=180
export QT_QPA_FONTDIR=/usr/share/fonts/ttf
export QT_QPA_PLATFORM=eglfs

/usr/bin/GWUsbDaemon &
/usr/bin/GangWriter &
