#!/bin/sh
clang *.c *.h *.cpp /usr/local/lib/libplist.a /usr/local/lib/libxml2.a /usr/local/lib/liblzma.a -liconv /usr/local/lib/libimobiledevice.a /usr/local/lib/libusbmuxd.a /usr/local/lib/libirecovery.a -lz -framework IOKit -lc++ -framework CoreFoundation -lssl /usr/local/Cellar/openssl/1.0.2h_1/lib/libcrypto.a 
