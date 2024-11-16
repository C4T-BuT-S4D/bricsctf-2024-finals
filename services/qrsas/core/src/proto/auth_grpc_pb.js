// GENERATED CODE -- DO NOT EDIT!

"use strict";
var grpc = require("@grpc/grpc-js");
var auth_pb = require("./auth_pb.js");

function serialize_CreateTokenRequest(arg) {
  if (!(arg instanceof auth_pb.CreateTokenRequest)) {
    throw new Error("Expected argument of type CreateTokenRequest");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_CreateTokenRequest(buffer_arg) {
  return auth_pb.CreateTokenRequest.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_CreateTokenResponse(arg) {
  if (!(arg instanceof auth_pb.CreateTokenResponse)) {
    throw new Error("Expected argument of type CreateTokenResponse");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_CreateTokenResponse(buffer_arg) {
  return auth_pb.CreateTokenResponse.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_InvalidateTokenRequest(arg) {
  if (!(arg instanceof auth_pb.InvalidateTokenRequest)) {
    throw new Error("Expected argument of type InvalidateTokenRequest");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_InvalidateTokenRequest(buffer_arg) {
  return auth_pb.InvalidateTokenRequest.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_InvalidateTokenResponse(arg) {
  if (!(arg instanceof auth_pb.InvalidateTokenResponse)) {
    throw new Error("Expected argument of type InvalidateTokenResponse");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_InvalidateTokenResponse(buffer_arg) {
  return auth_pb.InvalidateTokenResponse.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_ListTokenResponse(arg) {
  if (!(arg instanceof auth_pb.ListTokenResponse)) {
    throw new Error("Expected argument of type ListTokenResponse");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_ListTokenResponse(buffer_arg) {
  return auth_pb.ListTokenResponse.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_ListTokensRequest(arg) {
  if (!(arg instanceof auth_pb.ListTokensRequest)) {
    throw new Error("Expected argument of type ListTokensRequest");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_ListTokensRequest(buffer_arg) {
  return auth_pb.ListTokensRequest.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_ValidateTokenRequest(arg) {
  if (!(arg instanceof auth_pb.ValidateTokenRequest)) {
    throw new Error("Expected argument of type ValidateTokenRequest");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_ValidateTokenRequest(buffer_arg) {
  return auth_pb.ValidateTokenRequest.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

function serialize_ValidateTokenResponse(arg) {
  if (!(arg instanceof auth_pb.ValidateTokenResponse)) {
    throw new Error("Expected argument of type ValidateTokenResponse");
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_ValidateTokenResponse(buffer_arg) {
  return auth_pb.ValidateTokenResponse.deserializeBinary(
    new Uint8Array(buffer_arg),
  );
}

var TokenServiceService = (exports.TokenServiceService = {
  createToken: {
    path: "/TokenService/CreateToken",
    requestStream: false,
    responseStream: false,
    requestType: auth_pb.CreateTokenRequest,
    responseType: auth_pb.CreateTokenResponse,
    requestSerialize: serialize_CreateTokenRequest,
    requestDeserialize: deserialize_CreateTokenRequest,
    responseSerialize: serialize_CreateTokenResponse,
    responseDeserialize: deserialize_CreateTokenResponse,
  },
  validateToken: {
    path: "/TokenService/ValidateToken",
    requestStream: false,
    responseStream: false,
    requestType: auth_pb.ValidateTokenRequest,
    responseType: auth_pb.ValidateTokenResponse,
    requestSerialize: serialize_ValidateTokenRequest,
    requestDeserialize: deserialize_ValidateTokenRequest,
    responseSerialize: serialize_ValidateTokenResponse,
    responseDeserialize: deserialize_ValidateTokenResponse,
  },
  invalidateToken: {
    path: "/TokenService/InvalidateToken",
    requestStream: false,
    responseStream: false,
    requestType: auth_pb.InvalidateTokenRequest,
    responseType: auth_pb.InvalidateTokenResponse,
    requestSerialize: serialize_InvalidateTokenRequest,
    requestDeserialize: deserialize_InvalidateTokenRequest,
    responseSerialize: serialize_InvalidateTokenResponse,
    responseDeserialize: deserialize_InvalidateTokenResponse,
  },
  listTokens: {
    path: "/TokenService/ListTokens",
    requestStream: false,
    responseStream: false,
    requestType: auth_pb.ListTokensRequest,
    responseType: auth_pb.ListTokenResponse,
    requestSerialize: serialize_ListTokensRequest,
    requestDeserialize: deserialize_ListTokensRequest,
    responseSerialize: serialize_ListTokenResponse,
    responseDeserialize: deserialize_ListTokenResponse,
  },
});

exports.TokenServiceClient =
  grpc.makeGenericClientConstructor(TokenServiceService);
