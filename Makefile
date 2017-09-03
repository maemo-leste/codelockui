all: libcodelockui.so.1.0.0

libcodelockui.so.1.0.0: codelockui.c codelockui.h clui-code-dialog.c clui-code-dialog.h
	$(CC) -Wall -fPIC $(CFLAGS) $(shell pkg-config --cflags --libs glib-2.0 gtk+-2.0 libosso dbus-1 dbus-glib-1 hildon-1) --std=gnu99 -shared -Wl,-soname=libcodelockui.so.1 $^ -o $@

clean:
	$(RM) libcodelockui.so.1.0.0

install:
	install -d "$(DESTDIR)/usr/lib/"
	install -m 644 libcodelockui.so.1.0.0 "$(DESTDIR)/usr/lib/"
	ln -s libcodelockui.so.1.0.0 "$(DESTDIR)/usr/lib/libcodelockui.so.1"
	ln -s libcodelockui.so.1 "$(DESTDIR)/usr/lib/libcodelockui.so"
	install -d "$(DESTDIR)/usr/include/"
	install -m 644 codelockui.h "$(DESTDIR)/usr/include/"
	install -m 644 clui-code-dialog.h "$(DESTDIR)/usr/include/"
	install -d "$(DESTDIR)/usr/lib/pkgconfig"
	install -m 644 codelockui.pc "$(DESTDIR)/usr/lib/pkgconfig/"
