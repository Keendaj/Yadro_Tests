FROM gcc:12 AS builder

RUN apt-get update && apt-get install -y cmake

WORKDIR /src
COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release -j$(nproc)

FROM ubuntu:22.04

WORKDIR /app

COPY --from=builder /src/build/DataCollectorBackend .

CMD ["./DataCollectorBackend"]