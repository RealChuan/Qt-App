#!/bin/sh
# 提权启动入口：/opt/Qt-App 下的可执行文件在 usr/bin（FHS 布局）
pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY /opt/Qt-App/usr/bin/Qt-App
