#include <iostream>
#include <string>
#include <errno.h>
#include <time.h>

#include <WinSock2.h>
#include <WS2tcpip.h>//getaddrinfo inet_ntop

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int tcp_listen(const char *host, const char *service, const int listen_num = 5)
{
	int listenfd, ret;
	const char on = 1;
	struct addrinfo hints, *res, *ressave;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;//SOCK_DGRAM;//SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;

	if (0 != (ret = getaddrinfo(host, service, &hints, &res)))
	{
		cout << "getaddrinfo error: " << gai_strerrorA(ret) << endl;
		return -1;
	}

	ressave = res;
	while (NULL != res)
	{
		if (-1 == (listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)))
		{
			cout << "create socket error: " << strerror(errno) << endl;
			res = res->ai_next;
			continue;
		}

		if (-1 == setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
		{
			cout << "setsockopt error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}

		int ipv6only = 0;
		if (setsockopt(listenfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only)) != 0) {
			cout << "set ipv6only failed!";
			continue;
		}

		if (-1 == bind(listenfd, res->ai_addr, res->ai_addrlen))
		{
			cout << "bind error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}

		if (-1 == listen(listenfd, listen_num))
		{
			cout << "listen error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}

		break;
	}

	freeaddrinfo(ressave);

	if (NULL == res)
		return -1;

	return listenfd;
}

int get_addrinfo(const struct sockaddr *addr, string &ip, uint16_t &port)
{
	void *numeric_addr = NULL;
	char addr_buff[INET6_ADDRSTRLEN];

	if (AF_INET == addr->sa_family)
	{
		numeric_addr = &((struct sockaddr_in*)addr)->sin_addr;
		port = ntohs(((struct sockaddr_in*)addr)->sin_port);
	}
	else if (AF_INET6 == addr->sa_family)
	{
		numeric_addr = &((struct sockaddr_in6*)addr)->sin6_addr;
		port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
	}
	else
	{
		return -1;
	}

	//printf("family:%d\n", addr->sa_family);
	if (NULL != inet_ntop(addr->sa_family, numeric_addr, addr_buff, sizeof(addr_buff)))
		ip = addr_buff;
	else
		return -1;

	return 0;
}

inline std::string get_remote_addr(int sock) {
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);

	if (!getpeername(sock, (struct sockaddr *)&addr, &len)) {
		char ipstr[NI_MAXHOST];

		if (!getnameinfo((struct sockaddr *)&addr, len, ipstr, sizeof(ipstr),
			nullptr, 0, NI_NUMERICHOST)) {
			return ipstr;
		}
	}

	return std::string();
}

int main(int argc, char *argv[])
{
	int listenfd, connfd;
	struct sockaddr_storage cliaddr;
	socklen_t len = sizeof(cliaddr);
	time_t now;
	char buff[128];

	if (2 == argc) //指定端口
		listenfd = tcp_listen(NULL, argv[1]);
	else if (3 == argc) //指定本地IP和端口
		listenfd = tcp_listen(argv[1], argv[2]);
	else
	{
		cout << "usage: " << argv[0] << " [<hostname/ipaddress>] <service/port>" << endl;
		return -1;
	}

	if (listenfd < 0)
	{
		cout << "call tcp_listen error" << endl;
		return -1;
	}

	while (true)
	{
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);

		string ip = "";
		uint16_t port = 0;
		get_addrinfo((struct sockaddr*)&cliaddr, ip, port);
		cout << "client " << ip << "|" << port << " login" << endl;

		auto clientip = get_remote_addr(connfd);
		cout << "clientip " << clientip << endl;

		now = time(NULL);
		snprintf(buff, sizeof(buff) - 1, "%.24s", ctime(&now));
		send(connfd, buff, strlen(buff), 0);
		closesocket(connfd);
	}

	closesocket(listenfd);
	return 0;
}





#ifdef WIN32
class WSInit
{
public:
	WSInit()
	{
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);
	}

	~WSInit() { WSACleanup(); }
};

static WSInit wsinit_;
#endif
