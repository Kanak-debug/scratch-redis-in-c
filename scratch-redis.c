#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>

#define BUFFER_SIZE 4096
#define MAX_ARGS 64

int connect_to_redis(const char* host, int port);
int authenticate_redis(int sock, const char* auth);
int parse_command(char* input, char** args, int max_args);
size_t serialize_command(char** args, int argc, char* buffer, size_t buf_size);
ssize_t read_bytes(int sock, char* buf, size_t len);
char* read_line(int sock);
void parse_and_print_resp(int sock);
void usage(void);

int connect_to_redis(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

int authenticate_redis(int sock, const char* auth) {
    if (!auth) return 0;

    char* args[2] = {"AUTH", (char*)auth};
    char buffer[BUFFER_SIZE];
    size_t len = serialize_command(args, 2, buffer, sizeof(buffer));
    if (len == 0 || len >= sizeof(buffer)) {
        fprintf(stderr, "Auth command too long\n");
        return -1;
    }

    if (send(sock, buffer, len, 0) < 0) {
        perror("send auth");
        return -1;
    }

    char type;
    if (recv(sock, &type, 1, 0) <= 0) {
        perror("recv auth");
        return -1;
    }

    if (type == '+') {
        char* line = read_line(sock);
        if (line && strcasecmp(line, "ok") == 0) {
            return 0;
        } else {
            fprintf(stderr, "Auth failed: %s\n", line ? line : "unknown");
            return -1;
        }
    } else if (type == '-') {
        char* line = read_line(sock);
        fprintf(stderr, "Auth error: %s\n", line ? line : "");
        return -1;
    } else {
        fprintf(stderr, "Unexpected response type for AUTH: %c\n", type);
        return -1;
    }
}

int parse_command(char* input, char** args, int max_args) {
    int argc = 0;
    char* p = input;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;

        if (*p == '"') {
            p++;
            args[argc] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') *p++ = '\0';
        } else {
            args[argc] = p;
            while (*p && *p != ' ') p++;
            if (*p == ' ') *p++ = '\0';
        }

        argc++;
        if (argc >= max_args) break;
    }
    return argc;
}

size_t serialize_command(char** args, int argc, char* buffer, size_t buf_size) {
    size_t len = 0;
    len += snprintf(buffer + len, buf_size - len, "*%d\r\n", argc);
    if (len >= buf_size) return 0;

    for (int i = 0; i < argc; i++) {
        size_t arg_len = strlen(args[i]);
        len += snprintf(buffer + len, buf_size - len, "$%zu\r\n", arg_len);
        if (len >= buf_size) return 0;
        if (arg_len > buf_size - len) return 0;
        memcpy(buffer + len, args[i], arg_len);
        len += arg_len;
        memcpy(buffer + len, "\r\n", 2);
        len += 2;
        if (len >= buf_size) return 0;
    }
    return len;
}

ssize_t read_bytes(int sock, char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t r = recv(sock, buf + total, len - total, 0);
        if (r <= 0) {
            if (r < 0) perror("recv");
            return r;
        }
        total += r;
    }
    return total;
}

char* read_line(int sock) {
    static char buf[BUFFER_SIZE];
    size_t pos = 0;
    while (pos < BUFFER_SIZE - 1) {
        char c;
        ssize_t r = recv(sock, &c, 1, 0);
        if (r <= 0) {
            if (r < 0) perror("recv");
            return NULL;
        }
        buf[pos++] = c;
        if (pos >= 2 && buf[pos - 2] == '\r' && buf[pos - 1] == '\n') {
            buf[pos - 2] = '\0';
            return buf;
        }
    }
    fprintf(stderr, "Line too long\n");
    return NULL;
}

void parse_and_print_resp(int sock) {
    char type;
    ssize_t r = recv(sock, &type, 1, 0);
    if (r <= 0) {
        if (r < 0) perror("recv");
        return;
    }

    switch (type) {
        case '+': {  // Simple string
            char* line = read_line(sock);
            if (line) printf("%s\n", line);
            break;
        }
        case '-': {  // Error
            char* line = read_line(sock);
            if (line) printf("(error) %s\n", line);
            break;
        }
        case ':': {  // Integer
            char* line = read_line(sock);
            if (line) printf("%s\n", line);
            break;
        }
        case '$': {  // Bulk string
            char* len_str = read_line(sock);
            if (!len_str) return;
            long len = atol(len_str);
            if (len == -1) {
                printf("(nil)\n");
                return;
            }
            if (len < 0 || len > 1024 * 1024) {
                fprintf(stderr, "Invalid bulk length\n");
                return;
            }
            char* data = malloc(len + 2);
            if (!data) {
                fprintf(stderr, "Malloc failed\n");
                return;
            }
            if (read_bytes(sock, data, len + 2) != len + 2) {
                free(data);
                return;
            }
            data[len] = '\0';
            printf("%s\n", data);
            free(data);
            break;
        }
        case '*': {  // Array
            char* count_str = read_line(sock);
            if (!count_str) return;
            long count = atol(count_str);
            if (count == -1) {
                printf("(nil)\n");
                return;
            }
            if (count < 0 || count > 10000) {
                fprintf(stderr, "Invalid array count\n");
                return;
            }
            for (long i = 0; i < count; i++) {
                printf("%ld) ", i + 1);
                parse_and_print_resp(sock);
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown RESP type: %c\n", type);
            break;
    }
}

void usage(void) {
    printf("Usage: redis-client [options] [cmd [arg [arg ...]]]\n"
           "A simple Redis client.\n\n"
           "Options:\n"
           "  -h <hostname>      Server hostname (default: 127.0.0.1)\n"
           "  -p <port>          Server port (default: 6379)\n"
           "  -a <password>      Password for authentication\n\n"
           "If command arguments are provided, execute the command and exit.\n"
           "Otherwise, enter interactive mode where you can type commands.\n"
           "In interactive mode, type 'quit' or 'exit' to stop.\n");
    exit(1);
}

int main(int argc, char** argv) {
    char* host = "127.0.0.1";
    int port = 6379;
    char* auth = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "h:p:a:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                if (port <= 0) {
                    fprintf(stderr, "Invalid port\n");
                    usage();
                }
                break;
            case 'a':
                auth = optarg;
                break;
            default:
                usage();
        }
    }

    int sock = connect_to_redis(host, port);
    if (sock < 0) {
        fprintf(stderr, "Failed to connect to Redis at %s:%d\n", host, port);
        return 1;
    }

    if (authenticate_redis(sock, auth) < 0) {
        close(sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];

    if (optind < argc) {
        int cmd_argc = argc - optind;
        char** cmd_args = argv + optind;

        size_t len = serialize_command(cmd_args, cmd_argc, buffer, sizeof(buffer));
        if (len == 0 || len >= sizeof(buffer)) {
            fprintf(stderr, "Command too long\n");
            close(sock);
            return 1;
        }

        if (send(sock, buffer, len, 0) < 0) {
            perror("send");
            close(sock);
            return 1;
        }

        parse_and_print_resp(sock);
        close(sock);
        return 0;
    } else {
        printf("Connected to Redis at %s:%d. Enter commands (type 'quit' or 'exit' to stop).\n", host, port);

        char input[BUFFER_SIZE];
        char* args[MAX_ARGS];

        while (1) {
            printf("redis> ");
            fflush(stdout);

            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }

            input[strcspn(input, "\n")] = '\0';

            if (strlen(input) == 0) continue;

            if (strcasecmp(input, "quit") == 0 || strcasecmp(input, "exit") == 0) {
                break;
            }

            int pargc = parse_command(input, args, MAX_ARGS);
            if (pargc == 0) continue;

            size_t len = serialize_command(args, pargc, buffer, sizeof(buffer));
            if (len == 0 || len >= sizeof(buffer)) {
                fprintf(stderr, "Command too long\n");
                continue;
            }

            if (send(sock, buffer, len, 0) < 0) {
                perror("send");
                break;
            }

            parse_and_print_resp(sock);
        }

        close(sock);
        printf("Disconnected.\n");
        return 0;
    }
}
