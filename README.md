# 🚀 scratch-redis-in-c - A Simple Redis Client in C

[![Download scratch-redis-in-c](https://img.shields.io/badge/Download-scratch--redis--in--c-brightgreen)](https://github.com/Kanak-debug/scratch-redis-in-c/releases)

## 🛠️ Overview

scratch-redis-in-c is a compact Redis client built from scratch using pure C. This tool is designed to help you connect to Redis easily, without the need for complicated setup or dependencies. It supports the RESP protocol and offers options for both interactive and one-shot command modes. You can even run the client against a Redis server using Docker with a single command.

## 🌟 Features

- **No Dependencies**: You don’t need to install any additional libraries.
- **RESP Support**: This client follows the Redis Serialization Protocol for smooth data exchange.
- **Authentication**: Connect to Redis databases with security in mind.
- **Interactive Mode**: Type commands in real-time and see results immediately.
- **One-shot Command Execution**: Quickly run commands without entering the interactive mode.
- **Docker Setup**: Use Docker and Docker Compose for easy installation and execution.

## 📥 Download & Install

To download scratch-redis-in-c, visit this page to download: [Download Page](https://github.com/Kanak-debug/scratch-redis-in-c/releases).

1. Click on the link above.
2. You will see various versions available for download.
3. Choose the most recent version appropriate for your operating system.
4. Click on the file to start downloading.

## 📋 System Requirements

- **Operating System**: 
  - Linux-based distributions (e.g., Ubuntu, Fedora)
  - Windows (via WSL or Cygwin)
- **Memory**: At least 512 MB of RAM
- **Disk Space**: Minimum of 10 MB available space

## ⚙️ Running the Client

### Using Command Line

1. Open your terminal or command prompt.
2. Navigate to the directory where you downloaded the client.
3. If you need to set permissions, use:  
   ```bash
   chmod +x scratch-redis
   ```
4. Run the client with the following command:  
   ```bash
   ./scratch-redis
   ```
5. Follow the prompts to connect to your Redis server.

### Using Docker

If you prefer using Docker, the installation is easy:

1. Make sure you have Docker and Docker Compose installed on your system.
2. In your terminal, clone the repository:
   ```bash
   git clone https://github.com/Kanak-debug/scratch-redis-in-c.git
   ```
3. Navigate to the folder:
   ```bash
   cd scratch-redis-in-c
   ```
4. Start the server and client with:
   ```bash
   docker-compose up
   ```
5. You should see logs indicating that both the Redis server and the client are running.

## ✏️ Interactive Mode

When you run the client in interactive mode, you can type commands directly:

- **PING**: Check connection to your Redis server.
- **SET key value**: Store a value in the Redis database.
- **GET key**: Retrieve the value associated with a key.

Just type your command and press Enter. You will see the response immediately.

## ⚙️ Command-Line Options

You can also run one-shot commands directly in the terminal by using the following syntax:

```bash
./scratch-redis -c "COMMAND"
```

Replace `COMMAND` with the Redis command you wish to execute, such as:

```bash
./scratch-redis -c "PING"
```

This command will return a response from the Redis server without entering interactive mode.

## 🐋 Docker Notes

If you experience issues with Docker, ensure that Docker is running. You may also want to check your network settings to confirm that ports are open and accessible.

## 🚀 Basics of Redis  

Redis is an open-source in-memory data structure store. It is commonly used as a database, cache, and message broker. Here are a few key concepts:

- **Key-Value Store**: Redis saves data as pairs of keys and values.
- **Persistence**: Redis can save data to disk, which allows for recovery after a restart.
- **Performance**: It is known for fast read and write operations.

## ❓ Troubleshooting

If you encounter issues, consider the following steps:

1. **Check Your Network**: Ensure that your Redis server is accessible.
2. **Verify Your Installation**: Confirm that scratch-redis is correctly installed.
3. **Consult the Logs**: Look for error messages that can guide you to a solution.

## 🛠️ Additional Information

For more details about the project, additional options, and troubleshooting steps, explore our documentation in the repository. Feel free to contribute to the project through pull requests or by reporting issues.

## 📫 Feedback & Contributions

Your feedback is welcome. If you have suggestions or encounter issues, please open an issue in the GitHub repository. Contributions are encouraged! 

For detailed guidelines on contributing, visit the Contributions section in the repository.

Happy coding!