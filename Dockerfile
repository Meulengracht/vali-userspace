# Use the incoming arch argument as the image base
ARG ARCH

# dockerfile for os applications builds
FROM valios/os-base-$ARCH:latest AS build

# Build configuration arguments, reuse ARCH
# CROSS_PATH must match what is set in the toolchain image
ARG CROSS_PATH=/usr/workspace/toolchain-out
ARG THEME_SECRET
ARG ARCH

# Setup required environmental variables
ENV CROSS=$CROSS_PATH
ENV VALI_CODE=$THEME_SECRET
ENV VALI_ARCH=$ARCH
ENV VALI_SDK_PATH=/usr/workspace/vali-sdk
ENV VALI_DDK_PATH=/usr/workspace/vali-ddk
ENV VALI_APPLICATION_PATH=/usr/workspace/vali-apps
ENV DEBIAN_FRONTEND=noninteractive

# Set the directory
WORKDIR /usr/workspace/vali-userspace

# Copy all repository files to image
COPY . .

# Build the userspace environment
RUN sed -i 's/\r$//' ./depends.sh && chmod +x ./depends.sh && ./depends.sh && \
    mkdir -p build && cd build && \
    cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/Vali.cmake .. && \
    make && make install && cd $VALI_APPLICATION_PATH && tar -czvf vali-apps.tar.gz * && \
    cp ./vali-apps.tar.gz /usr/workspace/vali-userspace/vali-apps.tar.gz

# Make an artifact stage specifically for building output with the command
# DOCKER_BUILDKIT=1 docker build --target artifact --output type=local,dest=. .
FROM scratch AS artifact
COPY --from=build /usr/workspace/vali-userspace/vali-apps.tar.gz /vali-apps.tar.gz
