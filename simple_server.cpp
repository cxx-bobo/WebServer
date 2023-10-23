#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc,char* argv[])
{
	if (argc <= 2)
	{
		printf( "Usage: %s ip_address portname\n", argv[0] );
		return 0;
	}

	const char* ip = argv[1];
	int port = atoi( argv[2] );
    
    /*socket()创建监听socket的文件描述符*/
	int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
	assert( listenfd >= 1 );

	struct sockaddr_in address;
	memset( &address, 0, sizeof( address ) );
	address.sin_family = AF_INET;
	address.sin_port = htons( port );
    /*将ip地址转化为网络字节序*/
	inet_pton( AF_INET, ip, &address.sin_addr );

	int ret = 0;
    /*bind()将文件描述符绑定到指定ip地址和端口*/
	ret = bind( listenfd, (struct sockaddr*)( &address ), 
				sizeof( address ) );
	assert( ret != -1 );

    /*listen()监听文件描述符，最多允许5个待处理请求排队等待*/
	ret = listen( listenfd, 5 );
	assert( ret != -1 );
	
	struct sockaddr_in client;
	socklen_t client_addrlength = sizeof( client );
    /*accept()建立连接*/
	int sockfd = accept( listenfd, (struct sockaddr*)( &address ), &client_addrlength );
				   	  	 
	char buf_size[1024] = {0};
	int recv_size = 0;
    /*recv()接收客户端数据*/
	recv_size = recv( sockfd, buf_size, sizeof( buf_size ) , 0);
	
	int send_size = 0;
    /*send()发送数据给客户端*/
	send_size = send( sockfd, buf_size , recv_size , 0 );
	
	close( sockfd );
	close( listenfd );

	return 0;
}

