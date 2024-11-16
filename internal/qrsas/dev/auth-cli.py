import grpc
from auth_pb2_grpc import TokenServiceStub
from auth_pb2 import CreateTokenRequest, Token

def main():
    channel = grpc.insecure_channel('localhost:8980')
    stub = TokenServiceStub(channel)

    resp = stub.CreateToken(CreateTokenRequest(user_id='100', token=Token(token='rAnD0MByTeS', name='haha')))
    print(resp)

    resp = stub.CreateToken(CreateTokenRequest(user_id='97', token=Token(token='rAnD0MByTeSS', name='haha')))
    print(resp)

if __name__ == '__main__':
    main()