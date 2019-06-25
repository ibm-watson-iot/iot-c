FROM alpine:latest

RUN apk add gcc linux-headers libc-dev
RUN mkdir -p /opt/ibm/cpsutil/bin
RUN mkdir -p /opt/ibm/cpsutil/lib

RUN chmod +x /opt/ibm/cpsutil/bin/cpsutil

ENTRYPOINT ["/opt/ibm/cpsutil/bin/cpsutil"]
