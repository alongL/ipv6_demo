#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
 
 
using namespace std;
 
int tcp_listen(const char *host, const char *service, const int listen_num = 5)
{
	int listenfd, ret;
	const int on = 1;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;
 
	if (0 != (ret = getaddrinfo(host, service, &hints, &res)))
	{
		cout << "getaddrinfo error: " << gai_strerror(ret) << endl;
		return -1;
	}
 
	ressave = res;
	while(NULL != res)
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
			close(listenfd);
			res = res->ai_next;
			continue;
		}
 
		if (-1 == bind(listenfd, res->ai_addr, res->ai_addrlen))
		{
			cout << "bind error: " << strerror(errno) << endl;
                        close(listenfd);
                        res = res->ai_next;
                        continue;
		}
 
		if (-1 == listen(listenfd, listen_num))
		{
			cout << "listen error: " << strerror(errno) << endl;
                        close(listenfd);
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
 
int get_addrinfo(const struct sockaddr *addr, string &ip, in_port_t &port)
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
 
	printf("family:%d\n", addr->sa_family);
	if (NULL != inet_ntop(addr->sa_family, numeric_addr, addr_buff, sizeof(addr_buff)))
		ip = addr_buff;
	else
		return -1;
 
	return 0;
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
		in_port_t port = 0;
		get_addrinfo((struct sockaddr*)&cliaddr, ip, port);
		cout << "client " << ip << "|" << port << " login" << endl;
 
		now = time(NULL);
		snprintf(buff, sizeof(buff) - 1, "%.24s", ctime(&now));
		write(connfd, buff, strlen(buff));
		close(connfd);
	}
 
	close(listenfd);
	return 0;
}
