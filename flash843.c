/** @file flash843.c
 * @brief brief description 
 * @author zhaoyou.pei@kuwo.cn
 * @date Friday, January 08, 2016
 * @version 0.01
 * gcc -o flash843 flash843.c -Wall -O2 -levent_core
 * gcc -o flash843 flash843.c -Wall -O2 -Wl,-Bstatic -levent_core -Wl,-Bdynamic -lrt
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/listener.h>

const char *g_req = "<policy-file-request/>";
const int g_reqlen = 22;
const char *g_ack = "<?xml version=\"1.0\"?><!DOCTYPE cross-domain-policy SYSTEM \"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\"><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>";
const int g_acklen = 202;

void cb_read(evutil_socket_t fd, short what, void *arg)
{
    if(what & EV_READ)
    {
        char buf[128];
        int len = recv(fd, buf, 127, 0);
    #ifdef _DEBUG
        buf[len] = 0;
        fprintf(stdout, "fd:%d recv:%s", fd, buf);
    #endif
        if(len >= g_reqlen)
        {
            if(memcmp(buf, g_req, g_reqlen) == 0)
            {
                send(fd, g_ack, g_acklen, 0);
            }
        }
    }
    if(what & EV_TIMEOUT)
    {
    #ifdef _DEBUG
        fprintf(stderr, "fd:%d timeout\n", fd);
    #endif
    }
    close(fd);
}

void cb_accept(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr *addr, int len, void *ptr)
{
    struct event_base *base = evconnlistener_get_base(listener);
    struct timeval to = {30, 0};
    if(event_base_once(base, sock, EV_READ, cb_read, NULL, &to) != 0)
    {
    #ifdef _DEBUG
        fprintf(stderr, "event_base_once fail\n");
    #endif
        return ;
    }
}

int main(int argc, char **argv)
{
    int port = 843;
    struct event_base *base = event_base_new();
    if(!base)
    {
        fprintf(stderr, "event_base_new() fail\n");
        return -1;
    }
    
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(port);
    
    struct evconnlistener *listener = evconnlistener_new_bind(base, cb_accept, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr*)&sin, sizeof(sin));
    if(!listener)
    {
        fprintf(stderr, "evconnlistener_new_bind fail\n");
        return -1;
    }
    
    event_base_loop(base, 0);
    evconnlistener_free(listener);
    
    return 0;
}

