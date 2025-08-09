# Use an official GCC image
FROM gcc:latest

# Set the working directory
WORKDIR /app

# Copy all project files
COPY . .

# Compile server.cpp
RUN g++ server.cpp -pthread -o server

# Expose the TCP port (Railway usually assigns it to $PORT)
EXPOSE 3000

# Run the server with the assigned port
CMD ["./server"]
