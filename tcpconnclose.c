/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2017, Joyent, Inc.
 */

/*
 * tcpconnclose: test behavior when a program connects to a remote host and
 * immediately closes the connection with SO_LINGER = 0.  Much of this code is
 * ripped from https://github.com/davepacheco/experiment-tcpclosetest.
 */

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

static const char *ct_arg0;

static void usage(void);
static int parse_ipv4(const char *, struct sockaddr_in *);
static void log_time(void);

int
main(int argc, char *argv[])
{
	int sockfd;
	char *endp;
	unsigned long port;
	struct sockaddr_in addr;
	struct protoent *protop;
	struct linger nolinger;

	ct_arg0 = argv[0];

	if (argc != 3) {
		usage();
	}

	if (parse_ipv4(argv[1], &addr) != 0) {
		warnx("failed to parse IP address: %s\n", argv[1]);
		usage();
	}

	errno = 0;
	port = strtoul(argv[2], &endp, 10);
	if (errno == 0 && (*endp != '\0' || port >= UINT16_MAX)) {
		warnx("invalid port: \"%s\"", argv[2]);
		usage();
	}
	if (errno != 0) {
		warn("invalid port: \"%s\"", argv[2]);
		usage();
	}
	addr.sin_port = htons((uint16_t)port);

	log_time();
	(void) printf("connecting to %s port %lu\n", argv[1], port);

	protop = getprotobyname("tcp");
	if (protop == NULL) {
		errx(1, "protocol not found: \"tcp\"");
	}

	sockfd = socket(PF_INET, SOCK_STREAM, protop->p_proto);
	if (sockfd < 0) {
		err(1, "socket");
	}

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof (addr)) != 0) {
		err(1, "connect");
	}

	log_time();
	(void) printf("connected\n");

	nolinger.l_onoff = 1;
	nolinger.l_linger = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &nolinger,
	    sizeof (nolinger)) != 0) {
		err(1, "setsockopt");
	}

	log_time();
	(void) printf("teardown\n");
	(void) close(sockfd);
	return (0);
}

static void
usage(void)
{
	printf("usage: %s IPV4_ADDRESS PORT\n", ct_arg0);
	exit(2);
}

/*
 * Prints a timestamp to stdout (with no newline).  This is a prelude to
 * printing some other message.
 */
static void
log_time(void)
{
	int errsave;
	time_t nowt;
	struct tm nowtm;
	char buf[sizeof ("2014-01-01T01:00:00Z")];

	errsave = errno;
	time(&nowt);
	gmtime_r(&nowt, &nowtm);
	if (strftime(buf, sizeof (buf), "%FT%TZ", &nowtm) == 0) {
		err(1, "strftime failed unexpectedly");
	}

	(void) fprintf(stdout, "%s: ", buf);
	errno = errsave;
}

/*
 * Parse the given IP address and store the result into "addrp".  Returns -1 on
 * bad input.
 */
static int
parse_ipv4(const char *ip, struct sockaddr_in *addrp)
{
	char buf[INET_ADDRSTRLEN];

	(void) strlcpy(buf, ip, sizeof (buf));
	bzero(addrp, sizeof (*addrp));
	if (inet_pton(AF_INET, buf, &addrp->sin_addr) != 1) {
		return (-1);
	}

	addrp->sin_family = AF_INET;
	return (0);
}
