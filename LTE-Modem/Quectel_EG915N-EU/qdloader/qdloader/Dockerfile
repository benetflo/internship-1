FROM ubuntu:20.04

# Install cross-compilation tools for ARM64 (aarch64)
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    make \
    libusb-1.0-0-dev \
    file \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Set up cross-compilation environment
ENV CC=aarch64-linux-gnu-gcc
ENV CXX=aarch64-linux-gnu-g++
ENV CROSS_COMPILE=aarch64-linux-gnu-

# Modify Makefile for ARM64 cross-compilation
RUN sed -i 's/CC=${CROSS_COMPILE}gcc/CC=aarch64-linux-gnu-gcc/' Makefile

# Clean and build for ARM64
RUN make clean && make

# Verify the binary architecture
RUN file ./out/QDloader

# Create a volume mount point for extracting the binary
VOLUME ["/output"]

# Copy the binary to the output directory
RUN mkdir /output && cp ./out/QDloader /output/QDloader-arm64

# Show file info
RUN echo "Binary created: /output/QDloader-arm64" && \
    file /output/QDloader-arm64 && \
    ls -la /output/QDloader-arm64