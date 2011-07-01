
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

int main()
{
    struct addrinfo hints, *ai, *cur_ai;
    int port, fd = -1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[10];
    char hostname[] = "muzee11.serverroom.us";
    port = 6602;
    snprintf(portstr, sizeof(portstr), "%d", port);
    int ret = getaddrinfo(hostname, portstr, &hints, &ai);
    if(ret)
    {
        printf("die");
        return ;
    }
    printf("getaddrinfo !!!\n");

    cur_ai = ai;
    fd = socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
    
redo:
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    int timeout = 50;
    ret = connect(fd, cur_ai->ai_addr, cur_ai->ai_addrlen);
    if(ret < 0)
    {
        
        struct pollfd p = {fd, POLLOUT, 0};
        while(timeout--) {
            printf("count down: %d\n", timeout);
            ret = poll(&p, 1, 100);
            if (ret > 0)
                break;
        }
        
    }
    printf("connect ret: %d\n", ret);
    socklen_t optlen;
    optlen = sizeof(ret);
    getsockopt (fd, SOL_SOCKET, SO_ERROR, &ret, &optlen);
    if (ret != 0) {
        printf(
               "TCP connection to %s:%d failed: %s\n",
               hostname, port, strerror(ret));
        return ;
    }

    /* send http request */
    {
        struct pollfd p = {fd, POLLOUT, 0};
        ret = 0;
        while(timeout--) {
            printf("count down: %d\n", timeout);
            ret = poll(&p, 1, 100);
            if (ret > 0)
                break;
        }
        if(ret)
        {
            printf("ready to write\n");
            char http_req[]="GET / HTTP/1.1\r\n\r\n";
            ret = send(fd, http_req, strlen(http_req), 0);
            printf("http req => ret %d\n", ret);
            if(ret < 0)
            {
                printf("die at http req sent.");
                return 0;
            }
        }
    }
    
    /* read data */
    int i =0;
    for(i=0;i<10;i++)
    {
    
        uint8_t buf[1024];
        int size = 1024;
        {
            struct pollfd p = {fd, POLLIN, 0};
            ret = 0;
            while(timeout--) {
                printf("count down: %d\n", timeout);
                ret = poll(&p, 1, 100);
                if (ret > 0)
                    break;
            }
            if(ret)
            {
                printf("ready to read\n");
                memset(buf, 0, size);
                ret = recv(fd, buf, 1024, 0);
                printf("http response => ret %d\n", ret);
                if(ret < 0)
                {
                    printf("die at http req sent.");
                    return 0;
                }
                printf("resp: %s\n", buf);
            }
        }
    }



    return 0;

}
