package org.cbs.authrpc;


import java.sql.SQLException;
import java.util.HashMap;
import java.util.Optional;
import java.util.logging.Logger;

import auth.proto.Auth;
import auth.proto.TokenServiceGrpc;
import io.grpc.stub.StreamObserver;

public class TokenService extends TokenServiceGrpc.TokenServiceImplBase {
    private static final Logger logger = Logger.getLogger(TokenService.class.getName());
    private final TokenStorage tokenStorage;
    private final HashMap<String, String> tokenCache;

    public TokenService(TokenStorage tokenStorage) {
        super();
        this.tokenStorage = tokenStorage;
        tokenCache = new HashMap<>();
    }

    @Override
    public void createToken(Auth.CreateTokenRequest request, StreamObserver<Auth.CreateTokenResponse> responseObserver) {
        try {
            tokenStorage.storeToken(request.getToken().getToken(), request.getUserId(), request.getToken().getName());
        } catch (Exception e) {
            responseObserver.onError(e);
            return;
        }

        Auth.CreateTokenResponse response = Auth.CreateTokenResponse.newBuilder()
                .setToken(request.getToken()).build();
        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }


    private Optional<String> getTokenWithCache(String token) throws SQLException {
        if (tokenCache.containsKey(token)) {
            return Optional.of(tokenCache.get(token));
        }
        var userId = tokenStorage.getUserId(token);
        userId.ifPresent(s -> tokenCache.put(token, s));
        return userId;
    }

    @Override
    public void validateToken(Auth.ValidateTokenRequest request, StreamObserver<Auth.ValidateTokenResponse> responseObserver) {

        try {
            var userId = getTokenWithCache(request.getToken());
            var isValid = userId.isPresent();
            Auth.ValidateTokenResponse response = Auth.ValidateTokenResponse.newBuilder()
                    .setIsValid(isValid).setUserId(userId.orElse("")).build();
            responseObserver.onNext(response);
            responseObserver.onCompleted();
        } catch (Exception e) {
            responseObserver.onError(e);
        }
    }

    @Override
    public void invalidateToken(Auth.InvalidateTokenRequest request, StreamObserver<Auth.InvalidateTokenResponse> responseObserver) {
        tokenCache.remove(request.getToken());
        try {
            tokenStorage.deleteToken(request.getToken());
            responseObserver.onNext(Auth.InvalidateTokenResponse.newBuilder().build());
            responseObserver.onCompleted();
        } catch (Exception e) {
            responseObserver.onError(e);
        }
    }

    @Override
    public void listTokens(Auth.ListTokensRequest request, StreamObserver<Auth.ListTokenResponse> responseObserver) {
        try {
            var tokens = tokenStorage.listUserTokens(request.getUserId());
            Auth.ListTokenResponse.Builder responseBuilder = Auth.ListTokenResponse.newBuilder();
            for (var token : tokens) {
                responseBuilder.addTokens(Auth.Token.newBuilder().setName(token.name()).setToken(token.token()).build());
            }
            responseObserver.onNext(responseBuilder.build());
            responseObserver.onCompleted();
        } catch (Exception e) {
            responseObserver.onError(e);
        }
    }
}
