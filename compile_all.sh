#!/bin/bash

set -e
SILENT=true

TAG=${1:-$(git describe --tags --abbrev=0)}

# Create firmware output directory
FIRMWARE_DIR="firmware"
mkdir -p $FIRMWARE_DIR

# log which keyboard and keymap is compiled
function log_info() {
    echo "Compiled keyboard:$1 with keymap:$2"
}

# Copy firmware to output directory
function save_firmware() {
    local TARGET_NAME=$1
    if [ -f ".build/${TARGET_NAME}.bin" ]; then
        cp ".build/${TARGET_NAME}.bin" "${FIRMWARE_DIR}/"
        cp ".build/${TARGET_NAME}.hex" "${FIRMWARE_DIR}/" 2>/dev/null || true
        echo "Saved: ${FIRMWARE_DIR}/${TARGET_NAME}.bin"
    fi
}

for KEYMAP in "via" "default" "ryodeushii" "salty"; do
    for LAYOUT in "ansi" "iso"; do
        for KEYBOARD in "air75v2" "halo75v2" "halo96v2" "gem80"; do
            if [ ! -d "keyboards/nuphy/$KEYBOARD/$LAYOUT/keymaps/$KEYMAP" ] ; then
                continue
            fi
            CONFIG_H="keyboards/nuphy/$KEYBOARD/config.h"
            if [ ! -f $CONFIG_H ]; then
                CONFIG_H="keyboards/nuphy/$KEYBOARD/$LAYOUT/config.h"
            fi
            sed -i 's/put_version_here/'$TAG'/' $CONFIG_H
            if [ $KEYBOARD == "gem80" ]; then
                sed -i 's/WORK_MODE THREE_MODE/WORK_MODE USB_MODE/' $CONFIG_H
                TARGET="wired-$KEYBOARD-$LAYOUT-$KEYMAP-$TAG"
                qmk compile -kb nuphy/$KEYBOARD/$LAYOUT -km $KEYMAP -j 0 -c -e TARGET="$TARGET" -e SILENT=$SILENT && log_info $KEYBOARD $KEYMAP && save_firmware "$TARGET"

                sed -i 's/WORK_MODE USB_MODE/WORK_MODE THREE_MODE/' $CONFIG_H
                TARGET="threemode-$KEYBOARD-$LAYOUT-$KEYMAP-$TAG"
                qmk compile -kb nuphy/$KEYBOARD/$LAYOUT -km $KEYMAP -j 0 -c -e TARGET="$TARGET" -e SILENT=$SILENT && log_info $KEYBOARD $KEYMAP && save_firmware "$TARGET"
            else
                TARGET="$KEYBOARD-$LAYOUT-$KEYMAP-$TAG"
                qmk compile --compiledb -e TARGET="$TARGET" -kb nuphy/$KEYBOARD/$LAYOUT -km $KEYMAP -j 0 -c -e SILENT=$SILENT && log_info $KEYBOARD $KEYMAP && save_firmware "$TARGET"
            fi
            sed -i 's/'$TAG'/put_version_here/' $CONFIG_H
        done
    done
done

echo ""
echo "========================================"
echo "All firmware saved to: ${FIRMWARE_DIR}/"
ls -lh ${FIRMWARE_DIR}/*.bin 2>/dev/null || echo "No .bin files found"
echo "========================================"
