FROM debian:bookworm-slim

ENV LANG=C.UTF-8

# gdb ships /usr/share/gdb/python, which avr-gdb looks for at startup.
RUN apt-get update && apt-get install -y --no-install-recommends \
        avarice \
        avr-libc \
        avrdude \
        binutils-avr \
        ca-certificates \
        gcc-avr \
        gdb \
        gdb-avr \
        git \
        make \
        usbutils \
        picocom \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
