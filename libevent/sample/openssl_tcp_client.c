
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <event2/bufferevent_ssl.h>
#else
#define close(x) closesocket(x)
#endif

//#include <stdio.h>
#include <string.h>


#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"


#define CA_CERT_FILE "ca.crt"
#define CLIENT_CERT_FILE "server.crt"
#define CLIENT_KEY_FILE "server.key"


int main(int argc, char **argv)
{
    const char* address = SERVER_IP;
#ifdef WIN32
    //windows��ʼ�����绷��
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        printf("Error at WSAStartup()\n");
        exit(-1);
    }
    printf("Server Running in WONDOWS\n");
#else
    printf("Server Running in LINUX\n");
#endif
    
    if(argc > 1)
    {
        address = argv[1];
    }
    
    SSL_METHOD  *meth;
    SSL_CTX     *ctx;
    SSL         *ssl;
    
    int nFd;
    int nLen;
    char szBuffer[1024];
    
    SSLeay_add_ssl_algorithms();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    
    // ʹ��SSL V3,V2
    ctx = SSL_CTX_new (SSLv23_method());
    if( ctx == NULL)
    {
        printf("SSL_CTX_new error!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // Ҫ��У��Է�֤�飬��ʾ��Ҫ��֤�������ˣ�������Ҫ��֤��ʹ��  SSL_VERIFY_NONE
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    
    
    // ����CA��֤��
    printf("SSL_CTX_load_verify_locations start!\n");
    if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
    {
        printf("SSL_CTX_load_verify_locations error!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // �����Լ���֤��
    if(SSL_CTX_use_certificate_file(ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
    {
        printf("SSL_CTX_use_certificate_file error!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    //// �����Լ���˽Կ ����˽Կ��Ҫ���룬��˼��ÿ�����ӷ���������Ҫ����
    //����������Ҫ��֤�ͻ��˵���ݣ�����Ҫ�ͻ��˼���˽Կ�����ڴ˴�����ֻ��Ҫ��֤��������ݣ�����������Լ���˽Կ
    //printf("SSL_CTX_use_PrivateKey_file start!\n");
    //if(SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
    //{
    // printf("SSL_CTX_use_PrivateKey_file error!\n");
    // ERR_print_errors_fp(stderr);
    // return -1;
    //}
    
    //// �ж�˽Կ�Ƿ���ȷ
    //if(!SSL_CTX_check_private_key(ctx))
    //{
    // printf("SSL_CTX_check_private_key error!\n");
    // ERR_print_errors_fp(stderr);
    // return -1;
    //}
    
    // ��������
    nFd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(address);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    
    //���ӷ�����
    if(connect(nFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("connect\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // �����Ӹ���SSL
    ssl = SSL_new (ctx);
    if( ssl == NULL)
    {
        printf("SSL_new error!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    SSL_set_fd (ssl, nFd);
    if( SSL_connect (ssl) != 1)
    {
        printf("SSL_new error!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    
    // ���в���
    sprintf(szBuffer, "\nthis is from client+++++++++++++++client send to server");
    SSL_write(ssl, szBuffer, strlen(szBuffer));
    
    // �ͷ���Դ
    memset(szBuffer, 0, sizeof(szBuffer));
    nLen = SSL_read(ssl,szBuffer, sizeof(szBuffer));
    fprintf(stderr, "Get Len %d %s ok\n", nLen, szBuffer);
    
    SSL_free (ssl);
    SSL_CTX_free (ctx);
    close(nFd);
}
