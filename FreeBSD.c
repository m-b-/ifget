#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netdb.h>
#include <ifaddrs.h>

char *
ioctls(int n)
{
	switch (n) {
	case SIOCGIFADDR:
		return "SIOCGIFADDR";
	case SIOCGIFBRDADDR:
		return "SIOCGIFBRDADDR";
	case SIOCGIFNETMASK:
		return "SIOCGIFNETMASK";
	}

	return "UNKNOWN";
}

int
doifreq(struct ifreq *ifr, char *ifname, int ioctln)
{
	int s;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1) {
		perror("socket(AF_INET, SOCK_DGRAM, IPPROTO_IP");
		return -1;
	}

	memset(ifr, '\0', sizeof(ifr));
	strncpy(ifr->ifr_name, ifname, sizeof(ifr->ifr_name));

	if (ioctl(s, ioctln, ifr) == -1) {
		perror(ioctls(ioctln));
		return -1;
	}

	close(s);

	return 0;
}

/* no SIOCGIFHWADDR for FreeBSD */
int
getmac(int argc, char *argv[], char *buf, int n)
{
	struct ifaddrs *ifaddr, *ifa;
	unsigned char *p;
	int ipf;

	if (argc < 2) {
		fprintf(stderr, "%s: interface name expected\n", argv[0]);
		return -1;
	}

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return -1;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, argv[1]) == 0)
		if (ifa->ifa_addr->sa_family == AF_LINK) {
			p = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifa->ifa_addr));
			snprintf(buf, n, "%.2x:%2.x:%.2x:%.2x:%2.x:%.2x",
				p[0], p[1], p[2], p[3], p[4], p[5]);

			freeifaddrs(ifaddr);
			return 0;
		}
	}

	freeifaddrs(ifaddr);

badifname:
	fprintf(stderr, "%s: interface '%s' not found\n", argv[0], argv[1]);
	return -1;

}

/* ioctl version, IPv4 only */
int
getip2(int argc, char *argv[], char *buf, int n)
{
	struct ifreq ifr;

	if (argc < 2) {
		fprintf(stderr, "%s: interface name expected\n", argv[0]);
		return -1;
	}

	/* can't do IPv6 with ioctl */
	if (strcmp(argv[1], "-6") == 0)
		return -1;

	if (doifreq(&ifr, argv[1], SIOCGIFADDR) == -1)
		return -1;

	inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, buf, n);

	return 0;
}

/* getifaddrs version */
int
getip(int argc, char *argv[], char *buf, int n)
{
	struct ifaddrs *ifaddr, *ifa;
	char *ifname, *p;
	int ipf;

	ipf = AF_INET;

	if (argc < 2)
		goto noifname;

	if (strcmp(argv[1], "-6") == 0) {
		ipf = AF_INET6;
		if (argc-1 < 1)
			goto noifname;
		ifname = argv[2];
	} else
		ifname = argv[1];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return -1;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, ifname) == 0)
		if (ifa->ifa_addr->sa_family == ipf)
		if (getnameinfo(ifa->ifa_addr,
				(ipf == AF_INET) ?
					sizeof(struct sockaddr_in)
					: sizeof(struct sockaddr_in6),
				buf, n, NULL, 0, NI_NUMERICHOST) == -1) {
			perror("getnameinfo");
			freeifaddrs(ifaddr);
			return -1;
		} else {
			/* strip %ifname */
			if (ipf == AF_INET6) {
				p = strchr(buf, '%');
				*p = '\0';
			}
			freeifaddrs(ifaddr);
			return 0;
		}
	}

	freeifaddrs(ifaddr);

badifname:
	fprintf(stderr, "%s: interface '%s' not found\n", argv[0], ifname);
	return -1;

noifname:
	fprintf(stderr, "%s: interface name expected\n", argv[0]);
	return -1;
}

int
getmask(int argc, char *argv[], char *buf, int n)
{
	struct ifreq ifr;
	
	if (argc < 2) {
		fprintf(stderr, "%s: interface name expected\n", argv[0]);
		return -1;
	}

	if (doifreq(&ifr, argv[1], SIOCGIFNETMASK) == -1)
		return -1;

	inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, buf, n);

	return 0;
}

/* ioctl, apparently not working */
int
getbcast2(int argc, char *argv[], char *buf, int n)
{
	struct ifreq ifr;
	
	if (argc < 2) {
		fprintf(stderr, "%s: interface name expected\n", argv[0]);
		return -1;
	}

	if (doifreq(&ifr, argv[1], SIOCGIFBRDADDR) == -1)
		return -1;

	inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr, buf, n);

	return 0;
}

int
getbcast(int argc, char *argv[], char *buf, int n)
{
	struct ifaddrs *ifaddr, *ifa;
	char *ifname, *p;
	int ipf;

	ipf = AF_INET;

	if (argc < 2)
		goto noifname;

	if (strcmp(argv[1], "-6") == 0) {
		ipf = AF_INET6;
		if (argc-1 < 1)
			goto noifname;
		ifname = argv[2];
	} else
		ifname = argv[1];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return -1;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, ifname) == 0)
		if (ifa->ifa_addr->sa_family == ipf)
		if (getnameinfo(ifa->ifa_broadaddr,
				(ipf == AF_INET) ?
					sizeof(struct sockaddr_in)
					: sizeof(struct sockaddr_in6),
				buf, n, NULL, 0, NI_NUMERICHOST) == -1) {
			perror("getnameinfo");
			freeifaddrs(ifaddr);
			return -1;
		} else {
			/* strip %ifname */
			if (ipf == AF_INET6) {
				p = strchr(buf, '%');
				*p = '\0';
			}
			freeifaddrs(ifaddr);
			return 0;
		}
	}

	freeifaddrs(ifaddr);

badifname:
	fprintf(stderr, "%s: interface '%s' not found\n", argv[0], ifname);
	return -1;

noifname:
	fprintf(stderr, "%s: interface name expected\n", argv[0]);
	return -1;

}
