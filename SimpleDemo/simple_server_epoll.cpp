/*连接多个客户端，收发多条消息*/

/**
    IO多路复用：单线程或单进程同时监测若干个文件描述符是否可以执行IO操作的能力。
    三种方法：
        1.select
        2.poll
        3.epoll	 这里用的是epoll
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

/*指定epoll时间的最大数量*/
#define MAX_EVENTS_NUMBER 5

/*将文件描述符设置成非阻塞的*/
int set_non_blocking( int fd )
{
	int old_state = fcntl( fd, F_GETFL );
	int new_state = old_state | O_NONBLOCK;
	fcntl( fd, F_SETFL, new_state );

	return old_state;	
}

void addfd( int epollfd , int fd )
{
	epoll_event event;
	/*同时监听可读事件EPOLLIN和边沿触发模式ET*/
	event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
	/*将文件描述符添加到epoll监听列表中*/
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
	set_non_blocking( fd );	
}


int main( int argc , char* argv[] )
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
	
	epoll_event events[ MAX_EVENTS_NUMBER ];
	/*epoll_create()创建epoll实例*/
    int epollfd = epoll_create( 5 );
	assert( epollfd != -1);
	/*addfd()将文件描述符listenfd添加到epoll实例epollfd中*/
	addfd( epollfd, listenfd );

	while(1)
	{
		/*epoll_wait()在一段超时时间内等待一组文件描述符上的事件。如果检测到事件 就将所有就绪的事件从内核事件表中复制到它的第二个参数events指向的数组中*/
		int number = epoll_wait( epollfd, events, MAX_EVENTS_NUMBER, -1 );
		if( number < 0 )
		{
			printf( "epoll_wait failed\n" );
			return -1;
		}

		/*遍历events数组来处理每个事件*/
		for( int i = 0; i < number; ++i )
		{
			const auto& event = events[i];	//声明一个常量引用event，避免不必要的拷贝操作，以提高效率
			const auto eventfd = event.data.fd;

			/*如果有新的客户端连接请求*/
			if( eventfd == listenfd )
			{
				struct sockaddr_in client;
				socklen_t client_addrlength = sizeof( client );
				/*accept()建立连接，创建一个新的套接字 sockfd，用于与客户端通信，同时将客户端的地址信息存储在 client 结构体中*/
				int sockfd = accept( listenfd, ( struct sockaddr* )( &address ),
							   	     &client_addrlength );	
				/*addf()将新的客户端套接字sockfd添加到epoll监听列表中*/		
				addfd( epollfd, sockfd );			 
			}

			/*如果发生可读事件，即有数据可从套接字中读取*/
			else if( event.events & EPOLLIN )
			{
				char buf[1024] = {0};
				while(1)
				{
					memset( buf, '\0', sizeof( buf ) );
					/*recv()接收客户端数据*/
					int recv_size  = recv( eventfd, buf, sizeof( buf ), 0 );
					if( recv_size < 0 )
					{
						if( ( errno == EAGAIN ) || ( errno == EWOULDBLOCK ) )
						{
							/*当前没有数据可接收，跳出循环*/
							break;
						}
						printf(" sockfd %d,recv msg failed\n", eventfd );
						close( eventfd );
						break;
					}
					else if( recv_size == 0)
					{
						/*客户端断开连接，关闭eventfd，跳出循环*/
					   	close( eventfd );
						break;	
					}
					else
					{
						/*send()发送数据给客户端*/
						send( eventfd, buf, recv_size, 0 );
					}	
				}
			}
		}	
	}

	close( listenfd );

	return 0;
}
