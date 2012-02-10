while true; do
	echo "X: $(cat /sys/devices/platform/omap/tsc/ain7), Y: $(cat /sys/devices/platform/omap/tsc/ain3), Z: $(cat /sys/devices/platform/omap/tsc/ain1)"
	usleep 50000
done
