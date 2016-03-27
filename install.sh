#!/bin/sh

gcc *.c -o "img-server" -lm
cp img-server.sh /etc/init.d/img-server
update-rc.d img-server defaults
