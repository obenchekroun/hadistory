# launch our autostart apps (if we are on the correct tty and not in X)
if [ "`tty`" = "/dev/tty1" ] && [ -z "$DISPLAY" ] && [ "$USER" = "pi" ]; then
    bash "/home/pi/hadistory/autostart/autostart.sh"
fi
