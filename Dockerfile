FROM alpine:latest

RUN apk add --no-cache \
    gcc \
    musl-dev \
    make

WORKDIR /app
COPY scratch-redis.c .

RUN gcc -Wall -Wextra -std=c11 -O2 scratch-redis.c -o scratch-redis

CMD ["./scratch-redis", "-h", "redis", "-p", "6379"]
