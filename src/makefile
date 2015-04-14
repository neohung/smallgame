ifeq ($(OS),Windows_NT)
	LIBS=-Lwebsockets\lib\win32 -lwebsockets -lz
	INCS=-Iwebsockets\include\linux
	LIBS+=-Lgtk\lib\win32 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lpango-1.0 -lgtk-3 -lglib-2.0 -lgdk-3 -lgobject-2.0
	INCS+=-Igtk\include\gtk-3.0 -Igtk\include\glib-2.0 -Igtk\lib\win32\glib-2.0\include -Igtk\include\gdk-pixbuf-2.0 -Igtk\include\pango-1.0 -Igtk\include\cairo -Igtk\include\atk-1.0
	OBJS= server.exe client.exe main.exe
else
UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS=-Lwebsockets/lib/win32 -lwebsockets -lz
		INCS=-Iwebsockets/include/linux
		OBJS= server client main
    endif
endif	
CC=gcc

all:
	$(CC) server.c $(LIBS) $(INCS) -o server
	$(CC) client.c $(LIBS) $(INCS) -o client
	$(CC) main.c $(LIBS) $(INCS) -o main
clean:
	rm $(OBJS)
