# recastnavigation tests
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-I$(srcdir)/../../source \
	-I$(srcdir)/../../source/library \
	-I$(srcdir)/../../source/support \
	-I/usr/include/panda3d -I/usr/include/eigen3 \
	-DUT_DEBUG=

AM_CXXFLAGS = -O0 -g3 -Wall -Wno-reorder -fmessage-length=0 -std=c++11
AM_LDFLAGS = -L/usr/lib/x86_64-linux-gnu/panda3d

LDADD = -lp3framework -lpanda -lpandaexpress -lp3dtool -lp3dtoolconfig \
		-lp3pystub -lXxf86dga -lpython2.7 -lpandaphysics -lpthread

noinst_PROGRAMS = basic octree_orig

BUILT_SOURCES = data.h

substDataDir = sed -e 's|@sampledatadir[@]|$(srcdir)/../data|g'

data.h: $(srcdir)/data.h.in Makefile
	$(substDataDir) $(srcdir)/$@.in > $@
	
EXTRA_DIST = data.h.in

common_sources = \
	$(srcdir)/../../source/utOctree.cxx
	
#basic
basic_SOURCES = \
	basic.cpp

nodist_basic_SOURCES = $(common_sources)

#octree_orig
octree_orig_SOURCES = \
	octree_orig.cpp

nodist_octree_orig_SOURCES = $(common_sources)

CLEANFILES = data.h