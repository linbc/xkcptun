
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define closesocket close
#endif // !WIN32


#define CA_CERT_FILE "ca.crt"
#define SERVER_CERT_FILE "server.crt"
#define SERVER_KEY_FILE "server.key"

#define LISTEN_PORT 8080

SSL* CreateSSL(evutil_socket_t fd)
{
    SSL_CTX* ctx = NULL;  
    SSL* ssl = NULL; 
    ctx = SSL_CTX_new (SSLv23_method());
    if( ctx == NULL)
    {
        printf("SSL_CTX_new error!\n");
        return NULL;
    }

    // Ҫ��У��Է�֤��  
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  

    // ����CA��֤��  
    if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
    {
        printf("SSL_CTX_load_verify_locations error!\n");
        return NULL;
    }

    // �����Լ���֤��  
    if(SSL_CTX_use_certificate_file(ctx, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
    {
        printf("SSL_CTX_use_certificate_file error!\n");
        return NULL;
    }

    // �����Լ���˽Կ  
    if(SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
    {
        printf("SSL_CTX_use_PrivateKey_file error!\n");
        return NULL;
    }

    // �ж�˽Կ�Ƿ���ȷ  
    if(!SSL_CTX_check_private_key(ctx))
    {
        printf("SSL_CTX_check_private_key error!\n");
        return NULL;
    }

    // �����Ӹ���SSL  
    ssl = SSL_new (ctx);
    if(!ssl)
    {
        printf("SSL_new error!\n");
        return NULL;
    }

    SSL_set_fd (ssl, fd);  
    if(SSL_accept (ssl) != 1)
    {
        int icode = -1;
        int iret = SSL_get_error(ssl, icode);
        printf("SSL_accept error! code = %d, iret = %d\n", icode, iret);
        return NULL;
    }

    return ssl;
}


void socket_read_cb(evutil_socket_t fd, short events, void *arg)
{
    SSL* ssl = (SSL*)arg;
    char msg[4096];
    memset(msg, 0, sizeof(msg));  
    int nLen = SSL_read(ssl,msg, sizeof(msg));  
    fprintf(stderr, "Get Len %d %s ok\n", nLen, msg);  
    strcat(msg, "\n this is from server========server resend to client");  
    SSL_write(ssl, msg, strlen(msg));
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
    printf("do_accept\n");
    struct event_base *base = (struct event_base*)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0)
    {
        perror("accept");
    }
    else if (fd > FD_SETSIZE)
    {
		closesocket(fd);
    }
    else
    {
        SSL* ssl = CreateSSL(fd);
        struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
        //����̬�����Ľṹ����Ϊevent�Ļص�����
        event_assign(ev, base, fd, EV_READ | EV_PERSIST,
                 socket_read_cb, (void*)ssl);

    event_add(ev, NULL);
    }
}

void run(void)
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

    base = event_base_new();
    if (!base)
        return; /*XXXerr*/

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(8080);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);

#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("bind");
        return;
    }
    if (listen(listener, 16)<0)
    {
        perror("listen");
        return;
    }

    listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
    event_add(listener_event, NULL);

    event_base_dispatch(base);
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    run();

    return 0;
}
