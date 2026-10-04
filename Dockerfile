FROM fedora:latest AS build

RUN dnf install -y \
    gcc-c++ \
    cmake \
    make \
    && dnf clean all

WORKDIR /app

COPY mnemosyne-cache /app/mnemosyne-cache
COPY mnemosyne /app/mnemosyne

WORKDIR /app/mnemosyne-cache

RUN cmake -S . -B docker-build \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_MNEMOSYNE=ON \
    && cmake --build docker-build -j$(nproc)


FROM fedora:latest

RUN dnf install -y \
    libatomic \
    && dnf clean all

WORKDIR /app

COPY --from=build \
    /app/mnemosyne-cache/docker-build/mnemosyne-cache \
    /app/mnemosyne-cache

EXPOSE 6379

CMD ["./mnemosyne-cache"]