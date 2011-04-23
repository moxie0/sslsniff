/*
 * Copyright (c) 2002-2009 Moxie Marlinspike
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

// XXX define one of these through autoconf
//#define HAVE_PF 
#define HAVE_NETFILTER 

#include <arpa/inet.h>

#ifdef HAVE_NETFILTER
#include <limits.h>
#include <linux/netfilter_ipv4.h>
#endif

#ifdef HAVE_PF
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/pfvar.h>
#endif


#include "util/Destination.hpp"

int Destination::getOriginalDestination(boost::asio::ip::tcp::socket &socket,
					boost::asio::ip::tcp::endpoint &originalDestination)
{
#ifdef HAVE_NETFILTER
	struct sockaddr_in serverAddr;
	int fd   = (int)socket.native();
	int size = sizeof(serverAddr);

	if (getsockopt(fd, SOL_IP, SO_ORIGINAL_DST, &serverAddr, (socklen_t*)&size) < 0) {
		perror("Could not determine socket's original destination.");
		throw IndeterminateDestinationException();
	}

	originalDestination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(ntohl(serverAddr.sin_addr.s_addr)), 
							     ntohs(serverAddr.sin_port));
	return 1;
#elif defined(HAVE_PF)
	boost::asio::ip::tcp::endpoint le = socket.local_endpoint();
	boost::asio::ip::tcp::endpoint re = socket.remote_endpoint();
	static int fd = -1;
	struct pfioc_natlook nl;

	if (fd < 0) {
		fd = open("/dev/pf", O_RDONLY);
		if (fd >= 0) {
			fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
		}
	}
	if (fd < 0) {
		perror("PF open failed");
		throw IndeterminateDestinationException();
	}

	memset(&nl, 0, sizeof(struct pfioc_natlook));
	nl.saddr.v4.s_addr = htonl(re.address().to_v4().to_ulong());
	nl.sport = htons(re.port());
	nl.daddr.v4.s_addr = htonl(le.address().to_v4().to_ulong());
	nl.dport = htons(le.port());
	nl.af = AF_INET;
	nl.proto = IPPROTO_TCP;
	nl.direction = PF_OUT;

	if (ioctl(fd, DIOCNATLOOK, &nl)) {
		if (errno != ENOENT) {
			perror("PF lookup failed: ioctl(DIOCNATLOOK)");
			close(fd);
			fd = -1;
		}
		throw IndeterminateDestinationException();
	}

	if (nl.daddr.v4.s_addr == nl.rdaddr.v4.s_addr && nl.dport == nl.rdport) {
		/* no destination addr/port rewriting in place */
		throw IndeterminateDestinationException();
	}

	originalDestination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(ntohl(nl.rdaddr.v4.s_addr)), 
							     ntohs(nl.rdport));
	return 1;
#else
	throw IndeterminateDestinationException();
#endif
}

