FROM ubuntu:latest
RUN apt update
RUN apt-get install -y linux-libc-dev
RUN apt install build-essential
RUN mkdir /home/projet
COPY tsh /home/projet/tsh