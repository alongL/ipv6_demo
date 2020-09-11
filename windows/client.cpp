#include <iostream>
#include <string>
#include <errno.h>
#include <time.h>

#include <WinSock2.h>
#include <WS2tcpip.h>//getaddrinfo inet_ntop

#pragma comment(lib, "ws2_32.lib")

#define  close closesocket

using namespace std;

int tcp_connect(const char *host, const char *service)
{
	int sockfd, ret;
	struct addrinfo hints, *res, *ressave;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;

	if (0 != (ret = getaddrinfo(host, service, &hints, &res)))
	{
		cout << "getaddrinfo error: " << gai_strerror(ret) << endl;
		return -1;
	}

	ressave = res;
	while (NULL != res)
	{
		if (-1 == (sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)))
		{
			cout << "create socket error: " << strerror(errno) << endl;
			res = res->ai_next;
			continue;
		}

		if (-1 == connect(sockfd, res->ai_addr, res->ai_addrlen))
		{
			cout << "connect error: " << strerror(errno) << endl;
			close(sockfd);

			res = res->ai_next;
			continue;
		}


		printf("connect sucess.\n");
		break;
	}

	freeaddrinfo(ressave);

	if (NULL == res)
		return -1;

	return sockfd;
}

int main(int argc, char *argv[])
{
	int sockfd, n;
	char buff[128];
	struct sockaddr_storage cliaddr;

	if (3 != argc)
	{
		cout << "usage: " << argv[0] << " <hostname/ipaddress> <service/port>" << endl;
		return -1;
	}

	sockfd = tcp_connect(argv[1], argv[2]);
	if (sockfd < 0)
	{
		cout << "call tcp_connect error" << endl;
		return -1;
	}

	memset(buff, 0, sizeof(buff));
	while ((n = recv(sockfd, buff, sizeof(buff)-1, 0) > 0))
	{
		cout << buff << endl;
		memset(buff, 0, sizeof(buff));
	}
	close(sockfd);

	return 0;
}
