FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clangd \
    clang-format \
    clang-tidy \
    valgrind \
    git \
    vim \
    cmake \
    libgtest-dev \
    curl \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

RUN cmake -S /usr/src/googletest -B /tmp/gtest-build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /tmp/gtest-build \
 && cmake --install /tmp/gtest-build \
 && rm -rf /tmp/gtest-build

COPY . /src
WORKDIR /src

CMD ["bash"]
