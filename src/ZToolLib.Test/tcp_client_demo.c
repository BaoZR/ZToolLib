#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ZToolLib/ztl_evloop.h"
#include "ZToolLib/ztl_tcp_client.h"
#include "ZToolLib/ztl_times.h"
#include "ZToolLib/ztl_utils.h"


static int timer_handler(ztl_evloop_t* evloop, uint64_t timer_id, void* udata)
{
    fprintf(stderr, "client timer_handler id=%d, udata=%p\n", (int)timer_id, udata);
    return 0;
}
static int timer_finalizer(ztl_evloop_t* evloop, uint64_t timer_id, void* udata)
{
    // fprintf(stderr, "client timer_finalizer id=%d, udata=%p\n", (int)timer_id, udata);
    return 0;
}

static int _ev_handler(ztl_evloop_t* evloop, ztl_connection_t* conn, int events)
{
    int rv;
    char buf[1000] = "";
    rv = recv(conn->fd, buf, sizeof(buf) - 1, 0);
    fprintf(stderr, "_ev_handler rv=%d, buf=%s\n", rv, buf);
    return 0;
}

void tcp_client_demo_0(int argc, char* argv[])
{
    int rv;
    sockhandle_t fd;
    ztl_evloop_t* evloop;
    const char* ip = "127.0.0.1";
    uint16_t port = 13579;

    evloop = NULL;
    ztl_evloop_create(&evloop, 4);
    ztl_evloop_init(evloop);
    ztl_evloop_start(evloop);

    fd = create_socket(SOCK_STREAM);
    set_nonblock(fd, true);
    set_tcp_nodelay(fd, true);
    set_snd_buffsize(fd, 1024 * 1024);

    rv = net_connect_nonb(fd, ip, port, 3000);
    if (rv < 0) {
        fprintf(stderr, "connect failed");
    }

    rv = ztl_evloop_add(evloop, fd, ZEV_POLLIN, _ev_handler, NULL);
    fprintf(stderr, "ztl_evloop_add failed rv:%d\n", rv);

    while (1)
    {
        char buf[1000] = "";
        fprintf(stderr, "input cmd:\n");
        scanf("%s", buf);
        if (ztl_stricmp(buf, "exit") == 0) {
            break;
        }

        rv = send(fd, buf, (int)strlen(buf), 0);
        fprintf(stderr, "send rv=%d\n", rv);

        ztl_evloop_looponce(evloop, 1000);
    }

    ztl_evloop_del(evloop, fd, ZEV_POLLIN | ZEV_POLLOUT);
    close_socket(fd);
    ztl_evloop_stop(evloop);
    ztl_evloop_release(evloop);
    fprintf(stderr, "test_tcp_client end!");
}

//////////////////////////////////////////////////////////////////////////

void _tcp_client_on_connect(ztl_tcp_client_t* cli)
{
    fprintf(stderr, "_tcp_client_on_connect\n");
}

void _tcp_client_on_disconnect(ztl_tcp_client_t* cli, int reason)
{
    fprintf(stderr, "_tcp_client_on_disconnect\n");
}

int _tcp_client_on_read(ztl_tcp_client_t* cli, sockhandle_t fd, void* udata)
{
    fprintf(stderr, "_tcp_client_on_read\n");
    char buf[1000] = "";
    int  size;

    size = ztl_tcp_client_recv(cli, buf, sizeof(buf) - 1);
    fprintf(stderr, "read size=%d, buf=%s\n", size, buf);
    return 0;
}

void tcp_client_demo(int argc, char* argv[])
{
    int rv;
    uint16_t server_port = 13579;
    char server_ip[32] = "127.0.0.1";
    ztl_tcp_client_t* client = NULL;

    if (argc >= 3) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
        server_port = atoi(argv[2]);
    }

    fprintf(stderr, "tcp_client_demo connect to %s:%d\n", server_ip, server_port);

    ztl_tcp_client_create(&client);
    if (!client) {
        fprintf(stderr, "ztl_tcp_client_create failed!\n");
        return;
    }

    ztl_tcp_client_set_debug(client, 1);
    ztl_tcp_client_register(client, server_ip, server_port);
    ztl_tcp_client_reg_on_connect(client, _tcp_client_on_connect);
    ztl_tcp_client_reg_on_disconnect(client, _tcp_client_on_disconnect);
    ztl_tcp_client_reg_on_read(client, _tcp_client_on_read);

    ztl_tcp_client_set_nodelay(client);

    bool sync_mode = true;
    rv = ztl_tcp_client_init(client, sync_mode);

    while (1)
    {
        char buf[1000] = "";
        scanf("%s", buf);
        if (strcmp(buf, "exit") == 0)
            break;

        rv = ztl_tcp_client_send(client, buf, (int)strlen(buf));
        if (rv <= 0) {
            fprintf(stderr, "send buf failed rv=%d\n", rv);
        }

        if (sync_mode)
        {
            ztl_tcp_client_poll(client, 1000);
        }
    }

    ztl_tcp_client_stop(client);
    ztl_tcp_client_release(client);
    fprintf(stderr, "tcp_client_demo done!\n");
}
