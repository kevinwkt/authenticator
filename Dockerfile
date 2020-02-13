FROM ubuntu:latest
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install clang
RUN apt-get -y install make 
WORKDIR /authenticator
COPY ./ /authenticator
RUN make clean
RUN make
CMD ["./main"]
