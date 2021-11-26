#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>

#define BACKLOG 10
#define BUFF_LEN 1024

//functions
void sendall(int s, char *buf, int len) {
    int total = 0;
    int bytesleft = len;
    int n;

    while (total < len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) {
            break;
        }
        total += n;
        bytesleft -= n;
    }
}

//loads bytes directly into the buffer without adding any terminator at the end
//returns the number of bytes it loaded into the buffer
int buff_loader(char *buff, int size, FILE *stream) {
    char c;
    int i;
    for (i = 0; i < size; i++) {
        c = (char)fgetc(stream);
        if (c == EOF) {
            break;
        }
        *buff = c;
        buff++;
    }
    return i;
}

//creates a simple server which is bound to a port number and sends over one file
//also terminates the program that calls it
int create_server(int port, const char *file) {
    int listen_fd, comm_fd;
    struct sockaddr_in my_addr;
    pid_t pid;

    //Create the Socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        printf("Failed to create the socket at port %d\n", port);
        return -1;
    }

    //Bind it to a port
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
        printf("Failed to bind to port%d\n", port);
        return -1;
    };

    //Listen
    if (listen(listen_fd, BACKLOG) == -1) {
        printf("Failed to listen at port %d\n", port);
        return -1;
    }

    //accept incoming connection
    comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
    if (comm_fd == -1) {
        printf("failed to accept at port %d\n", port);
        return -1;
    }

    pid = fork();
    //child
    if (pid == 0) {
        close(listen_fd);

        //here we begin to send the message over
        //just like main() for the child process
        FILE *csv_file;
        char buff[BUFF_LEN];
        int r;

        csv_file = fopen(file, "r");
        if (csv_file == NULL) {
            printf("could not find %s\n", file);
            close(comm_fd);
            exit(0);
        }

        while (1) {
            r = buff_loader(buff, BUFF_LEN, csv_file);
            sendall(comm_fd, buff, r);

            if (r < BUFF_LEN) {
                break;
            }
        }

        assert(fclose(csv_file) == 0);
        close(comm_fd);
        exit(0);
    }
    //parent
    else {
        close(comm_fd);

        //reap dead children, if any
        int status;
        pid_t deadChild;
        do {
            deadChild = waitpid(-1, &status, 0);
        } while (deadChild > 0);
    }

    exit(0);
}


int main() {
    int ports[] = {8020, 8021, 8022, 8025, 8080, 8443};
    const char *files[6];
    files[0] = "../data/file1.csv";
    files[1] = "../data/file2.csv";
    files[2] = "../data/file3.csv";
    files[3] = "../data/file4.csv";
    files[4] = "../data/file5_bad.csv";
    files[5] = "../data/file9_bad.csv";
    int i, r;
    pid_t pids[6];


    for (i = 0; i < 6; i++) {
        pids[i] = fork();

        //child
        if (pids[i] == 0) {
            r = create_server(ports[i], files[i]);
            if (r == -1) {
                printf("child %d failed to create its server\n", i+1);
                exit(0);
            }

            else {
                printf("this should never be called but is a backup\n");
                exit(0);
            }
        }
    }

    //reap dead children, if any
    int status;
    pid_t deadChild;
    do {
        deadChild = waitpid(-1, &status, 0);
    } while (deadChild > 0);

    return 0;
}