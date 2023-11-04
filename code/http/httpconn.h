#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"
/*
进行读写数据并调用httprequest 来解析数据以及httpresponse来生成响应
*/
class HttpConn {
public:
    HttpConn();
    ~HttpConn();
    
    void init(int sockFd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void Close();
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;
    bool process();

    // 写的总长度
    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;  // 用于标记是否使用 ET 模式
    static const char* srcDir;  //指定存放资源文件的根目录
    static std::atomic<int> userCount;  // 原子，支持锁 (记录当前连接数)
    
private:
   
    int fd_;  // 连接的文件描述符
    struct  sockaddr_in addr_;  // 连接的地址信息

    bool isClose_;  // 标记连接是否已关闭
    
    int iovCnt_;  // iovec 数组的长度
    struct iovec iov_[2];  // iovec 数组，用于写入数据
    
    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpRequest request_;  // HttpRequest 对象，用于处理客户端的 HTTP 请求
    HttpResponse response_;  // HttpResponse 对象，用于生成 HTTP 响应
};


#endif //HTTP_CONN_H