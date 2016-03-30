#!/bin/bash
while [ true ]; do
	/home/administrateur/public-imgserver/img-server
	if [ $? = 0 ]; then # Normal quit : Maybe the admin pressed Ctrl+C. Wait before restarting.
		echo "Please press Ctrl+C (again). If you don't, the server will restart in 5 seconds."
		sleep 5
	else
		echo "Crash! Restarting the server immediatly..."
	fi
done
