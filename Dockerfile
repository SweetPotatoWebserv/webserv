FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    valgrind \
    vim \
    git \
    cmake \
    make \
    clang \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

RUN cmake -S /usr/src/googletest -B /tmp/gtest-build -DCMAKE_CXX_STANDARD=17 \
 && cmake --build /tmp/gtest-build --config Release \
 && cmake --install /tmp/gtest-build

COPY . /src
WORKDIR /src
CMD ["bash"]
