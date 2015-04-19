ifeq ($(OS),Windows_NT)
	LIBS=-Lwebsockets\lib\win32 -Lzlib\lib\win32 -lwebsockets -lz
	INCS=-Iwebsockets\include\linux
	LIBS+= -Lgtk\lib\win32 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lpango-1.0 -lgtk-3 -lglib-2.0 -lgdk-3 -lgobject-2.0
	INCS+= -Igtk\include\win32\gtk-3.0 -Igtk\include\win32\glib-2.0 -Igtk\lib\win32\glib-2.0\include -Igtk\include\win32\gdk-pixbuf-2.0 -Igtk\include\win32\pango-1.0 -Igtk\include\win32\cairo -Igtk\include\win32\atk-1.0
	OBJS=server.exe client.exe main.exe
	COPY_PREBUILD=copy bin\win32
	RM=del /Q
	RMDIR=rmdir /s /q
else
UNAME_S := $(shell uname -s)
	#-Lgtk/lib/linux/64-bit
    ifeq ($(UNAME_S),Linux)
    LIBS=-Lwebsockets/lib/linux/64-bit -lwebsockets -lz -lcurl -lpng
	INCS=-Iwebsockets/include/linux
	#LIBS+= -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lgmodule-2.0
	#INCS+= -pthread -Igtk/include/linux/gtk-3.0 -Igtk/include/linux/at-spi2-atk/2.0 -Igtk/include/linux/at-spi-2.0 -Igtk/include/linux/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -Igtk/include/linux/gtk-3.0 -Igtk/include/linux/gio-unix-2.0/ -Igtk/include/linux/mirclient -Igtk/include/linux/mircommon -Igtk/include/linux/cairo -Igtk/include/linux/pango-1.0 -Igtk/include/linux/harfbuzz -Igtk/include/linux/pango-1.0 -Igtk/include/linux/atk-1.0 -Igtk/include/linux/cairo -Igtk/include/linux/pixman-1 -Igtk/include/linux/freetype2 -Igtk/include/linux/libpng12 -Igtk/include/linux/gdk-pixbuf-2.0 -Igtk/include/linux/libpng12 -Igtk/include/linux/glib-2.0 -Igtk/include/linux/glib-2.0/include
	LIBS+= `pkg-config --libs gtk+-2.0`
	INCS+= `pkg-config --cflags gtk+-2.0`
	OBJS= server client main
	COPY_PREBUILD=cp -rf bin/linux/64-bit/*
	RM=rm -f
	RMDIR=rm -rf
    endif
endif	
CC=gcc

all:
	-$(RMDIR) build
	-mkdir build
	#$(CC) server.c $(LIBS) $(INCS) -o build/server
	#$(CC) client.c $(LIBS) $(INCS) -o build/client
	#$(CC) main.c $(LIBS) $(INCS) -o build/main
	$(CC) test1.c $(LIBS) $(INCS) -o build/test1
	$(COPY_PREBUILD) build
clean:
	-$(RMDIR) build
	
