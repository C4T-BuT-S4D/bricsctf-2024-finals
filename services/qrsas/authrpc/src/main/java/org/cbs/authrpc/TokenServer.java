package org.cbs.authrpc;

import io.grpc.Server;
import io.grpc.ServerBuilder;
import io.grpc.protobuf.services.ProtoReflectionServiceV1;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

class TokenServer {
    private static final Logger logger = Logger.getLogger(TokenServer.class.getName());

    private final int listenPort;
    private final Server server;

    TokenServer(int port, TokenService service) {
        listenPort = port;
        server = ServerBuilder.forPort(port).addService(service).addService(ProtoReflectionServiceV1.newInstance()).build();
    }

    void start() throws IOException {
        server.start();
        logger.info("Server started, listening on " + listenPort);
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                // Use stderr here since the logger may have been reset by its JVM shutdown hook.
                System.err.println("*** shutting down gRPC server since JVM is shutting down");
                try {
                    TokenServer.this.stop();
                } catch (InterruptedException e) {
                    e.printStackTrace(System.err);
                }
                System.err.println("*** server shut down");
            }
        });
    }

    private void stop() throws InterruptedException {
        if (server != null) {
            server.shutdown().awaitTermination(30, TimeUnit.SECONDS);
        }
    }

    public void blockUntilShutdown() throws InterruptedException {
        if (server != null) {
            server.awaitTermination();
        }
    }
}