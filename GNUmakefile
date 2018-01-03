#
# Makefile for tcpconnclose
#

CLEANFILES	 = tcpconnclose
CFLAGS		+= -Werror -Wall -Wextra -fno-omit-frame-pointer
LDFLAGS		+= -lgen -lsocket -lnsl

tcpconnclose: tcpconnclose.c
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^

clean:
	rm -f $(CLEANFILES)
