#!/bin/sh
PID=$(ps aux | grep arduino_sim | grep -v grep | awk '{print $2}' | head -n 1)
echo "$1" >> /proc/$PID/cwd/___client_events

