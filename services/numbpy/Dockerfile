FROM golang:1.23.2-alpine AS server-builder

WORKDIR /build
COPY go.mod .
COPY go.sum .
RUN go mod download

COPY cmd cmd
COPY internal internal
RUN go build -o numb cmd/numb/main.go

FROM python@sha256:b52e97a85736bc3a6bfb081ec3f2a5b828233120b2138080af1761e504b06529
WORKDIR /app
COPY requirments.txt ./
RUN pip install -r requirments.txt
COPY run_python.py ./
COPY numb.cpython-312-x86_64-linux-gnu.so ./
COPY --from=server-builder /build/numb numb_server
CMD ["./numb_server"]
