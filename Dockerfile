FROM gcc:latest

WORKDIR /app

# Copy all files into the container
COPY . .

# Compile the C++ server
RUN g++ server.cpp -pthread -o server

# Expose port (match the port your server.cpp uses)
EXPOSE 5000

# Start the server
CMD ["./server"]
