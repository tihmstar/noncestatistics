AM_CXXFLAGS = $(libplist_CFLAGS) $(libimobiledevice_CFLAGS) $(libirecovery_CFLAGS)
AM_LDFLAGS = $(libplist_LIBS) $(libimobiledevice_LIBS) $(libirecovery_LIBS)

bin_PROGRAMS	= noncestatistics
noncestatistics_CXXFLAGS = $(AM_CXXFLAGS)
noncestatistics_CFLAGS = $(AM_CXXFLAGS)
noncestatistics_LDADD = $(AM_LDFLAGS)
noncestatistics_SOURCES = common.c dfu.c idevicerestore.c normal.c recovery.c stats.cpp main.cpp
