#/bin/bash

FIRMWARE=$1
OTAHOSTIP="'192.168.4.1'"
USBPORT="/dev/ttyUSB0"
USBSPEED="115200"

PYTHON="$(command -v python)"
if [ -x "$(command -v python3)" ]; then
  echo 'Selecting python3' >&2
fi

showHelp () {
  echo ""
  echo "usage: ./install.sh [binary]"
  echo "usage: ./install.sh [command] [binary] [option] [option]"
  echo ""
  echo "Examples"
  echo ""
  echo "Install via default USB port:"
  echo ""
  echo "./install.sh xxx_2021XXXXrevXXX.bin"
  echo ""
  echo "Install via WiFi:"
  echo ""
  echo "./install.sh ota xxx.bin"
  echo "./install.sh ota xxx.bin 192.168.4.103"
  echo ""
  echo "Install via USB:"
  echo ""
  echo "./install.sh usb xxx.bin"
  echo "./install.sh usb xxx.bin /dev/ttyUSB3"
  echo "./install.sh usb xxx.bin /dev/ttyUSB3 1500000"
  echo ""
  echo ""
}

flashOTA () {
  $PYTHON ./system/espota.py --port=3232 --auth=AIROBOT --debug --progress -i $2 -f $1
}

flash () {
  $PYTHON ./system/esptool.py --chip esp32 --port $2 --baud $3 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 system/bootloader_dio_40m.bin 0x8000 system/partitions.bin 0xe000 system/boot_app0.bin 0x10000 $1
}

erase_flash () {
    if ! [[ -z "$2" ]]; then
      USBPORT="$2"
    fi
    $PYTHON ./system/esptool.py --port $USBPORT erase_flash
}

printParams() {
  echo ""
  echo "###############################################"
  echo "firmware: $3"
  echo "update via $1: $2"
  echo "speed: $4"
  echo "###############################################"
  echo ""
}

##########################################
#  M  A  I  N
##########################################

if [ "$1" = "" ]; then
  showHelp
  exit 1
fi

case "$1" in
  help)
    showHelp
    ;;

  ota)
    if ! [[ -z "$3" ]]; then
      OTAHOSTIP="$3"
    fi
    FIRMWARE="$2"
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "IP" $OTAHOSTIP $FIRMWARE
    else
      showHelp
    fi

    flashOTA "$2" "$OTAHOSTIP"

    ;;

  usb)
    if ! [[ -z "$3" ]]; then
      USBPORT="$3"
    fi
    if ! [[ -z "$4" ]]; then
      USBSPEED="$4"
    fi
    FIRMWARE="$2"
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "USB" $USBPORT $FIRMWARE $USBSPEED
    else
      showHelp
    fi

    flash "$2" "$USBPORT" "$USBSPEED"

    ;;

erase)
    erase_flash "$1"
    ;;

  *)
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "USB" $USBPORT $FIRMWARE $USBSPEED
      flash "$FIRMWARE" "$USBPORT" "$USBSPEED"
    else
      showHelp
    fi
    ;;
esac
