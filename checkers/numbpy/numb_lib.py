from typing import NamedTuple, List, Optional

import requests
import websocket
from checklib import *

PORT = 7117


class CheckMachine:
    @property
    def url(self):
        return f"http://{self.c.host}:{self.port}"

    def __init__(self, checker: BaseChecker):
        self.c = checker
        self.port = PORT

    def register(
        self, session: requests.Session, username: str, password: str, status: Status
    ):
        resp = session.post(
            f"{self.url}/api/register",
            json={
                "login": username,
                "password": password,
            },
        )

        resp = self.c.get_json(resp, "invalid response on register", status)
        self.c.assert_eq(type(resp), dict, "invalid response on register", status)
        if resp.get("status") != "success":
            self.c.cquit(status, f"could not register: {resp.get('message', '')}")
        self.c.assert_eq(
            type(resp.get("message")), dict, "invalid response on register", status
        )
        jwt = resp["message"].get("token")
        self.c.assert_eq(type(jwt), str, "invalid response on register", status)

        session.headers["Authorization"] = f"Bearer {jwt}"

    def login(
        self, session: requests.Session, username: str, password: str, status: Status
    ):
        resp = session.post(
            f"{self.url}/api/login",
            json={
                "login": username,
                "password": password,
            },
        )

        resp = self.c.get_json(resp, "invalid response on login", status)
        self.c.assert_eq(type(resp), dict, "invalid response on login", status)
        if resp.get("status") != "success":
            self.c.cquit(status, f"could not login: {resp.get('message', '')}")
        self.c.assert_eq(
            type(resp.get("message")), dict, "invalid response on login", status
        )
        jwt = resp["message"].get("token")
        self.c.assert_eq(type(jwt), str, "invalid response on login", status)

        session.headers["Authorization"] = f"Bearer {jwt}"

    def put_repl(self, session: requests.Session, code: str, status: Status) -> int:
        resp = session.put(
            f"{self.url}/api/repl",
            json={
                "code": code,
            },
        )

        resp = self.c.get_json(resp, "invalid response on PutRepl", status)
        self.c.assert_eq(type(resp), dict, "invalid response on PutRepl", status)
        if resp.get("status") != "success":
            self.c.cquit(status, f"could not PutRepl: {resp.get('message', '')}")
        self.c.assert_eq(
            type(resp.get("message")), dict, "invalid response on PutRepl", status
        )
        uid = resp["message"].get("id")
        self.c.assert_eq(type(uid), int, "invalid response on PutRepl", status)

        return uid

    def get_repl_code(
        self, session: requests.Session, repl_id: int, status: Status
    ) -> str:
        resp = session.get(f"{self.url}/api/repl/{repl_id}")

        resp = self.c.get_json(resp, "invalid response on GetReplCode", status)
        self.c.assert_eq(type(resp), dict, "invalid response on GetReplCode", status)
        if resp.get("status") != "success":
            self.c.cquit(status, f"could not GetReplCode: {resp.get('message', '')}")
        self.c.assert_eq(
            type(resp.get("message")), dict, "invalid response on GetReplCode", status
        )
        code = resp["message"].get("code")
        self.c.assert_eq(type(code), str, "invalid response on GetReplCode", status)

        return code

    def edit_repl_code(
        self, session: requests.Session, repl_id: int, code: str, status: Status
    ) -> str:
        resp = session.patch(f"{self.url}/api/repl/{repl_id}", json={"code": code})

        resp = self.c.get_json(resp, "invalid response on EditReplCode", status)
        self.c.assert_eq(type(resp), dict, "invalid response on EditReplCode", status)
        if resp.get("status") != "success":
            self.c.cquit(status, f"could not EditReplCode: {resp.get('message', '')}")

    def run_repl(
        self, session: requests.Session, repl_id: int, status: Status
    ) -> websocket.WebSocket:
        ws = websocket.WebSocket()
        ws.connect(
            f"ws://{self.c.host}:{PORT}/api/run/{repl_id}",
            header={"Authorization": session.headers["Authorization"]},
        )
        ws.settimeout(7)
        return ws
