// Copyright 2024 QMK
// Copyright 2024 Su (@isuua)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#if RAW_ENABLE

#    include "quantum.h"
#    include "wireless.h"
#    include "usb_endpoints.h"
#    include "usb_main.h"

void replaced_hid_send(uint8_t *data, uint8_t length) {

    if (length != RAW_EPSIZE) {
        return;
    }

#    ifdef WECAT4_DIAG
    // 진단 빌드: 무선 모드에서도 USB 가 살아 있으면 응답을 USB 로 보낸다.
    if (get_transport() == TRANSPORT_USB || USB_DRIVER.state == USB_ACTIVE) {
#    else
    if (get_transport() == TRANSPORT_USB) {
#    endif
        send_report(USB_ENDPOINT_IN_RAW, data, length);
    } else {
        md_send_raw(data, length);
    }
}

#endif
