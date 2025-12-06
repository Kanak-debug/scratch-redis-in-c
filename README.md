# scratch-redis-in-c

A minimal Redis client written from scratch in pure C, with zero dependencies beyond the standard C library and POSIX sockets.
Includes Docker + Docker Compose setup to run the client against a Redis server container.

---

## 📌 Overview

`scratch-redis-in-c` is a lightweight Redis command-line client that:

* Connects directly to a Redis server using raw TCP sockets
* Implements Redis RESP protocol manually (no hiredis or external libs)
* Supports:
  * Simple commands (`PING`, `SET`, `GET`, …)
  * Bulk strings
  * Arrays
  * Integers
  * Authentication (`AUTH password`)
* Works interactively or in one-shot (non-interactive) mode
* Runs standalone or inside Docker

This is ideal for **learning Redis protocol**, **building minimal tooling**, or **embedding a tiny client in C applications**.

---

## 📁 Project Structure

```
scratch-redis-in-c/
│
├── scratch-redis.c       # The C source code (Redis client)
├── Dockerfile            # Docker image for the client
├── docker-compose.yml    # Redis + client orchestration
└── README.md             # Documentation
```

---

## 🚀 Building the Client (Native)

Compile using GCC:

```bash
gcc scratch-redis.c -o scratch-redis
```

Or with recommended flags:

```bash
gcc -Wall -Wextra -std=c11 -O2 scratch-redis.c -o scratch-redis
```

Run:

```bash
./scratch-redis PING
```

---

## 🐳 Docker Setup

### **Dockerfile**

The client container is based on Alpine Linux, compiles the C program inside the image, and runs it automatically:

```Dockerfile
FROM alpine:latest

RUN apk add --no-cache \
    gcc \
    musl-dev \
    make

WORKDIR /app
COPY scratch-redis.c .

RUN gcc -Wall -Wextra -std=c11 -O2 scratch-redis.c -o scratch-redis

CMD ["./scratch-redis", "-h", "redis", "-p", "6379"]
```

---

## 🐳 Docker Compose

This setup launches:

* A Redis server (`redis:7-alpine`)
* Your custom C client container compiled from source

```yaml
services:
  redis:
    image: redis:7-alpine
    container_name: my-redis-server
    ports:
      - "6379:6379"
    command: redis-server --save "" --appendonly no

  redis-client:
    build:
      context: .
      dockerfile: Dockerfile
    container_name: my-redis-client
    depends_on:
      - redis
    tty: true
    stdin_open: true
```

Start the environment:

```bash
docker compose up -d --build
```

---

## 🧪 Using the Client Inside Docker

### **One-shot command**

```bash
docker compose run --rm redis-client ./scratch-redis -h redis PING
# → PONG
```

### **With password**

```bash
docker compose run --rm redis-client ./scratch-redis -h redis -a mysecretpassword SET foo bar
```

### **Interactive mode**

```bash
docker compose run --rm redis-client sh
# inside container:
./scratch-redis -h redis
```

Then use commands like:

```
redis> SET name "Max Base"
redis> GET name
redis> INCR counter
redis> EXIT
```

---

## 📝 Command-Line Options

```
-h <hostname>     Redis server hostname (default: 127.0.0.1)
-p <port>         Redis server port (default: 6379)
-a <password>     Authenticate using AUTH <password>
```

Examples:

```bash
./scratch-redis -h 127.0.0.1 -p 6379 PING
./scratch-redis -h 10.1.0.5 SET hello world
./scratch-redis -a secret PASS
```

If no command is provided, the client enters **interactive mode**:

```bash
./scratch-redis
```

---

## 🔧 Features Implemented Internally

This project manually implements core Redis RESP protocol components:

### **✔ TCP socket handling**

* `socket()`, `connect()`, `recv()`, `send()`, `close()`

### **✔ RESP Serialization**

The client manually builds messages like:

```
*2\r\n$4\r\nPING\r\n$4\r\nPONG\r\n
```

### **✔ RESP Parsing**

Supports:

* Simple Strings (`+OK`)
* Errors (`-ERR something`)
* Integers (`:1000`)
* Bulk Strings (`$5\r\nhello`)
* Arrays (`*3 ...`)

All printing behavior mimics the official Redis CLI style.

---

## 🧠 Example Session

```
$ ./scratch-redis -h localhost
Connected to Redis at localhost:6379. Enter commands:

redis> PING
PONG

redis> SET lang C
OK

redis> GET lang
C

redis> LRANGE mylist 0 -1
1) hello
2) world

redis> exit
Disconnected.
```

---

## 🎯 Why This Project?

This Redis client is intentionally minimal — made for:

* Learning Redis internals
* Understanding RESP wire protocol
* Low-level experimentation
* Writing tiny CLI tools
* Academic or research purposes
* A base for building your own advanced Redis client in C

---

## 📄 License

MIT License

Copyright © 2025 **Seyyed Ali Mohammadiyeh (Max Base)**
