//
// Created by Георгий Розовский on 02/06/2019.
//

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <alloca.h>
#include "utils.cpp"


int const buf_const_size = 2048;

/*
 * first arg - name of server to connect
 * second arg - name of file
 */
int main(int argc, char **argv) {

    if (argc < 3) {
        my_error("Incorrect number of args\n");
    }

    char *file_name = argv[2];
    struct sockaddr_un server_addr;
    int socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_ < 0) {
        my_error("Socket error\n");
    }

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, argv[1]);
    if (connect(socket_, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        my_error("Connection error\n");
    }
    write_str(1, "Connected to server\n");

    write_str(socket_, file_name, "Error was occurred while sending name of file\n");

    char buffer[buf_const_size];
    struct iovec iovec_;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    int fd;

    iovec_.iov_base = buffer;
    iovec_.iov_len = buf_const_size;

    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iovec_;
    msg.msg_iovlen = 1;

    cmsg = (cmsghdr *) alloca(sizeof(struct cmsghdr) + sizeof(fd));
    cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
    msg.msg_control = cmsg;
    msg.msg_controllen = cmsg->cmsg_len;

    if (recvmsg(socket_, &msg, 0) < 0) {
        my_error("Recvmsg error\n");
    }
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
    write_str(1, "Input your text.\nAfter that it will be reverse and all letters will be replaced with capital letters and input in file\n"
                 "To finish work send '!close'\n"
                 "To finish work of client and server send !close_server\n");
    bool checker = true;
    while (checker) {
        char answer[buf_const_size];
        scanf("%s", answer);
        if (strcmp(answer, "!close") == 0 || strcmp(answer, "!close_server") == 0) {
            checker = false;
        }
        write_str(fd, answer, "Error was occurred while writing to file\n");
        if (!checker){
            write_str(1, "Success!\nr");
        }
    }
    remove(file_name);
    write_str(1, "Closed and removed.\n");
    close(socket_);
    return 0;
}