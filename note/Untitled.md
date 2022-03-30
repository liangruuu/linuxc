# 套接字-字节序问题

网络套接字socket

* 讨论：跨主机的传输要注意的问题

* 报式套接字

* 流式套接字



1. 字节序问题：

* 大端存储：低地址处放高字节
* 小端存储：低地址处放低字节

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330184259975.png" alt="image-20220330184259975" style="zoom:80%;" />

不管是文件传输还是IO实现，总是低地址处的数据先出去，高地址处的数据后出去。这就有问题了，比如说此时A发出了一个数据到B，比如A是小端格式存储，但是B当做大端格式接收，那么从A发出的00-00-00-05数据被B端接收，因为虽然解析格式会变，但是数据是不会变的，数据被B接收仍然是00-00-00-05，但是因为是大端格式存储，所以会被解析成05-00-00-00

解决方式是不区分大端存储还是小端存储，而是区分主机字节序(host)和网络字节序(network)，现在有一组函数来进行这个序列的转换：`__to__`，当前PC机上的内容要通过网络传输到对端的话，那么当前的数据就是主机序转网络序，如果数据是从网络接收过来的话就选择网络序转主机序

```markdown
htons / htonl: host to network 传输2字节数据 / 传输4字节数据

ntohs / ntohl: network to host 传输2字节数据 / 传输4字节数据
```

2. 对齐

```c
struct
{
    int i;
    float f;
    char ch;
}
```

编译器把结构体进行对齐的目的是加速节省当前的取址周期，凡是参与网络通信的结构体都要禁止对齐操作，所以在定义结构体的时候一定要告诉编译器不进行对齐，如果对齐的话会对协议数据解析产生影响

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330190450077.png" alt="image-20220330190450077" style="zoom:80%;" />

3. 类型长度问题：不同计算机环境下对于int、char、float等数据类型的长度定义不同，解决办法是用一些通用的数据类型，比如32位的有符号整型数int32_t，32位无符号整型数uint32_t，char型数据int8_t......

socket是什么：有很多网络协议比如IPV4、AX25....来完成网络协议族的指定，同时可以有多种不同的数据传输方式，比如说流式传输、数据包式传输.....，其实在不同的协议族当中对于不同的网络实现方式都会有支持，所以可以看到如果两两组合的话就会对程序员的工作造成巨大的压力，换一个协议/传输方式就得重新写一遍代码，我们借助一个机制把用什么样子的协议族和数据传输方式给指定起来，这个机制就是socket，而socket提供给用户的是一个抽象出来的文件描述符，也就是说只需要凭借一个文件描述符就能解决套接字传输的问题，因为socket本质上是一个文件描述符，所以之前讲的关于文件IO的操作都能适用于socket，对于一个文件描述符的操作无非就是打开关闭读写定位......

<img src="/home/liangruuu/.config/Typora/typora-user-images/image-20220330192256107.png" alt="image-20220330192256107" style="zoom:80%;" />

>NAME
>
>> socket - create an endpoint for communication
>
>SYNOPSIS
>
>> #include <sys/types.h>          /* See NOTES */
>> #include <sys/socket.h>
>>
>> int socket(int domain, int type, int protocol);
>
>1. int socket(int domain, int type, int protocol)：上一幅图中说到了socket其实是封装协议和网络实现方式的一个机制，所以在创建socket的时候一定会指定网络协议和数据传输方式；第一个参数domain其实指的就是协议族；第二个参数type指的是网络实现方式；第三个参数protocol指的是协议，比如用的是domain协议族中的protocol协议来进行type类型的传输，返回值是socket的文件描述符
>
>
>
>DESCRIPTION
>
>```shell
># domain
>
>Name         Purpose                                    Man page
>AF_UNIX      Local communication                        unix(7)
>AF_LOCAL     Synonym for AF_UNIX
>AF_INET      IPv4 Internet protocols                    ip(7)
>AF_AX25      Amateur radio AX.25 protocol               ax25(4)
>AF_IPX       IPX - Novell protocols
>AF_APPLETALK AppleTalk                                  ddp(7)
>
>
>
># type
># 流式
>SOCK_STREAM     Provides  sequenced,  reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.		
>
># 报式
>SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
>
>
>```
>
>* sequenced,  reliable, two-way, connection-based byte streams：有序可靠指的是只要接收放能接受到数据就保证数据包中的内容以及内容顺序是正确的，但不是指的是不丢包，在网络传输过程中丢包是一定的；双工指的是通信双方都能主动发送和接收数据；基于连接指的是三次握手，点对点传输字节流数据，如果数据是以单字节为单位进行传输的话就相当于流式套接字，如果每个数据不是char型，而是一个结构体指针类型的话就相当于报式传输，是以数据分组格式传输的；
>* 字节流SOCK_STREAM的意思是数据没有特别严格的边界，比如作为发送方给对方发送100个字节数据，可以选择一次接收50个字节，并接受两次，也可以选择一次接收20个字节，并接收5次.....
>* 报式传输SOCK_DGRAM是要求的数据分组和完整性，以一个数据包里的格式进行发送，比如传输结构体，结构体中有姓名、年龄......每个结构体都和其他及饿哦固体有着非常清晰的边界



















