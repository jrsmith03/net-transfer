#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>


void* handle_con (void*);

int main(int argc, char** argv) {
    
    // Establish an IPV4 socket file descriptor using TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // First argument is the address for which to bind the socket to.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = atoi(argv[1]);
    struct in_addr sin_addr;
    sin_addr.s_addr = INADDR_ANY; 
    addr.sin_addr = sin_addr; 

    if (bind(sockfd, &addr, sizeof(addr)) == -1) {
        printf("Binding failed!\n");
    }
    
    // Now we want to make the sockfd a passive socket
    if (listen(sockfd, 255) == -1) {
        printf("Listen failed!\n");
    }
    printf("Server successfully established! Waiting for incoming connections...\n");
    // I'm not sure about this, but we might want to create a directory that is locked per thread so that there aren't races between client data.
    // But that might defeat the purpose of multithreading.

    // Loop indefinitely until the server recieves SIGINT
    switch (atoi(argv[2])) {
        // Support for the client writing directly to the system's stdout
        case 1:
            // socklen_t addr_len = sizeof(addr);
            // int client_fd = accept(sockfd, &addr, &addr_len);
            // printf("new client: %d\n", client_fd);
            // while(1) {
            //     fork();
            //     execl('/bin/bash', NULL);
            //     char in[1];
            //     read(client_fd, in, 1);
            //     printf ("%s",in);
            //     write(fileno(stdin), in, 1);
            // }
            // break;
        // Support for multithreaded transfer of files
        case 2:
            while (1) {
                socklen_t addr_len = sizeof(addr);
                int client_fd = accept(sockfd, &addr, &addr_len);
                if (client_fd != -1) {
                    pthread_t worker;
                    pthread_create(&worker, NULL, handle_con, &client_fd);
                }
            }
            break;
    }
    

    return 0;
}


void* handle_con (void* v_client_id) {
    int client_fd = *(int*)(v_client_id);
    if (client_fd == -1) {
        printf("Unable to accept connection \n");
        return;
    } else {
        printf("Worker ID %d; New client connected at %d\n", client_fd);


        // Create the file that the client is transferring.
        char file_name[32];
        read(client_fd, file_name, 32);
        char *size_char = malloc(sizeof(char) * 32);
        // Read the file size
        read(client_fd, size_char, 32);
        printf("Creating new file %s of size %s.\n",file_name, size_char);

        int transfer_fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (transfer_fd == -1) {
            printf("Unable to create a file to write to, aborting! Error: %s\n", strerror(errno));
            close(client_fd);
            return;
        }
        // Transfer the contents from the client. 
        char *file_mem = malloc(sizeof(char) * atoi(size_char)); 
        size_t bytes_read = read(client_fd, file_mem, atoi(size_char));
        while(bytes_read != atoi(size_char)) {
            int reanew = read(client_fd, file_mem + bytes_read, atoi(size_char) - bytes_read);
            if (reanew == -1) {
                printf("fatal error- read failed; error %s\n", strerror(errno));
                close(client_fd);
                return -1;
            }
            bytes_read += reanew;
        }
        size_t bytes_written = write(transfer_fd, file_mem, atoi(size_char));

        while (bytes_written != atoi(size_char)) {
            // printf("point in array: %s", file_mem[written]);
            int writnew = write(transfer_fd, file_mem + bytes_written, atoi(size_char) - bytes_written);
            if (writnew == -1) {
                printf("fatal error- write failed; error %s\n", strerror(errno));
                close(client_fd);
                return -1;
            }
            bytes_written += writnew;
        }

        if (bytes_written != atoi(size_char)) {
            printf("Error writing the new file.");
        }
        // char buf[1];
        // ssize_t numread = read(client_fd, buf, 1);

        // while(numread == 1) {
        //     size_t written = write(transfer_fd, buf, 1);
        //     if (written != 1) {
        //         printf("Unable to write byte error %d\n", errno);
        //     }
        //     numread = read(client_fd, buf, 1);
        // }
        printf("Client %d done transferring.\n", client_fd);
        close(client_fd);
    }
}