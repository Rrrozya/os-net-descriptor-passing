//
// Created by Георгий Розовский on 02/06/2019.
//

#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.cpp"


int const buf_const_size = 2048;

/*
 * first arg - name of server
 */
int main(int argc, char **argv) {

    if (argc < 2) {
        my_error("Incorrect number of args\n");
    }

    char *server_fake_id = argv[1];
    unlink(server_fake_id);

    int socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_ < 0) {
        my_error("Socket error\n");
    }

    int client_sockfd;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, server_fake_id);
    if (bind(socket_, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        my_error("Binding error\n");
    }

    listen(socket_, 5);
    write_str(1, "Waiting for client connection.......... \n");

    while (true) {
        ssize_t end_f;
        char buffer[buf_const_size];
        char file_piped[buf_const_size];
        int fd;

        socklen_t client_len = sizeof(client_addr);
        client_sockfd = accept(socket_,
                               (struct sockaddr *) &client_addr, &client_len);

        if (client_sockfd < 0) {
            my_error("Client socket fd error\n");
        }

        if ((end_f = read(client_sockfd, buffer, buf_const_size)) < 0) {
            my_error("Reading fd error\n");
        }

        buffer[end_f] = '\0';
        strncpy(file_piped, buffer, static_cast<size_t>(end_f + 1));

        if (mkfifo(file_piped, 0777)) {
            my_error("Making FIFO error\n");
        }

        if ((fd = open(file_piped, O_RDWR)) <= 0) {
            remove(file_piped);
            my_error("Fd opening error\n");
        }
        write_str(1, "File, which named, ");
        write_str(1, file_piped);
        write_str(1, " has been opened.\n");

        struct iovec iovec_;
        struct msghdr msg;
        struct cmsghdr *cmsg;

        iovec_.iov_base = file_piped;
        iovec_.iov_len = strlen(file_piped) + 1;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iovec_;
        msg.msg_iovlen = 1;

        cmsg = (struct cmsghdr *) alloca(sizeof(struct cmsghdr) + sizeof(fd));
        cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

        msg.msg_control = cmsg;
        msg.msg_controllen = cmsg->cmsg_len;

        if (sendmsg(client_sockfd, &msg, 0) != static_cast<ssize_t >(iovec_.iov_len)) {
            remove(file_piped);
            my_error("Sendmsg error\n");
        }

        close(client_sockfd);
        memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

        bool cont = true;
        while (true) {
            char buffer_2[buf_const_size];
            if ((end_f = read(fd, buffer_2, buf_const_size - 1)) < 0) {
                my_error("Reading error\n");
            }

            buffer_2[end_f] = '\0';
            if (strcmp("!close", buffer_2) == 0) {
                break;
            }

            if (strcmp("!close_server", buffer_2) == 0) {
                cont = false;
                break;
            }

            write_str(1, buffer_2);
            write_str(1, "\n");
            char answer[end_f + 1];
            answer[end_f] = '\0';
            for (int i = 0; i < end_f; i++) {
                if (buffer_2[end_f - 1 - i] >= 'a' && buffer_2[end_f - 1 - i] <= 'z')
                    answer[i] = static_cast<char>(buffer_2[end_f - 1 - i] - 32);
                else answer[i] = buffer_2[end_f - 1 - i];
            }
            write_str(1, answer);
            write_str(1, "\n");

        }
        write_str(1, "Closed<3\n");
        close(fd);
        if (!cont) {
            break;
        }
    }
    close(socket_);
    return 0;
}