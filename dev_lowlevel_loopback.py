#!/usr/bin/env python3

#
# Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# sudo pip3 install pyusb

import usb.core
import usb.util

# find our device
dev = usb.core.find(idVendor=0xcafe, idProduct=0x4014)

# was it found?
if dev is None:
    raise ValueError('Device not found')

# get an endpoint instance
cfg = dev.get_active_configuration()
intf = cfg[(0, 0)]

outep = usb.util.find_descriptor(
    intf,
    # match the first OUT endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

inep = usb.util.find_descriptor(
    intf,
    # match the first IN endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN)

assert inep is not None
assert outep is not None

test_string = "LED_ON"
#outep.write(test_string)
from_device = inep.read(64)

print("Device Says: {}".format(''.join([chr(x) for x in from_device])))
