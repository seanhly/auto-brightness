#!/bin/bash
BRIGHTNESS_VCP_CODE=10
brightness_data="$(sudo ddcutil getvcp $BRIGHTNESS_VCP_CODE)"
current_brightness="$(
	echo "$brightness_data" |
		grep -oP '(?<=current value = {,40})\d+'
)"
max_brightness="$(
	echo "$brightness_data" |
		grep -oP '(?<=max value = {,40})\d+'
)"
lat_lon="$(curl -s ipinfo.io/loc)"
zenith="$(sunposition -lat "${lat_lon%,*}" -lon "${lat_lon#*,}" |
	grep -Po '(?<=, zenith = [^,]{,40}, )[0-9.]+')"
echo $zenith $current_brightness |
	awk -v max_brightness="$max_brightness" '{
		zenith = $1 / 180
		current_brightness = $2 / max_brightness
		current_brightness = sprintf("%.2f", current_brightness)
		print zenith "," current_brightness
	}'

# Append to datapoints.csv
echo "$zenith $current_brightness" | awk -v max_brightness="$max_brightness" '
BEGIN {
	print "zenith,brightness" >> "datapoints.csv"
}
{
    zenith = $1 / 180
    current_brightness = $2 / max_brightness
    current_brightness = sprintf("%.2f", current_brightness)
    print zenith "," current_brightness >> "datapoints.csv"
}'
