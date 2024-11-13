#!/usr/bin/env python3

from typing import Tuple
import random
import secrets
import sys
import json
import struct

from checklib import *
import requests
import websocket
import numpy as np

from numb_lib import CheckMachine


EPS = 1e-9


def random_matrix(n: int, m: int) -> np.matrix:
    matrix = np.matrix(
        [[random.uniform(-1337, 1337) for _ in range(m)] for _ in range(n)]
    )
    return matrix


def random_invertible_matrix(n: int, m: int) -> np.matrix:
    while True:
        matrix = random_matrix(n, m)
        if abs(np.linalg.det(matrix)) > EPS:
            return matrix


def matrix_to_numb(matrix: np.matrix) -> str:
    n, m = matrix.shape
    return f"Matrix({[[float(matrix[i,j]) for j in range(m)] for i in range(n)]})"


def program_with_matrix_output(m: np.matrix, ops: int = 5) -> Tuple[str, str]:
    if ops == 0:
        return matrix_to_numb(m)

    choice = random.choice(["add", "sub", "scale", "mul"])
    if choice == "add":
        a = random_matrix(*m.shape)
        return f"({program_with_matrix_output(m - a, ops-1)} + {matrix_to_numb(a)})"
    elif choice == "sub":
        a = random_matrix(*m.shape)
        return f"({program_with_matrix_output(m + a, ops-1)} - {matrix_to_numb(a)})"
    elif choice == "scale":
        lamba = random.uniform(-1337, 1337)
        return f"({program_with_matrix_output(m / lamba,  ops-1)} * {lamba})"
    elif choice == "mul":
        a = random_invertible_matrix(*m.shape)
        return f"{program_with_matrix_output(m * (a ** -1), ops-1)}.dot({matrix_to_numb(a)})"


def program_with_string_output(output: str, ops: int = 5) -> str:
    m = random_invertible_matrix(len(output), len(output))
    m_inv = m**-1
    v_m = m.dot([ord(i) for i in output])
    return "\n".join(
        [
            "from numb import Matrix, Vector",
            f"v = {program_with_matrix_output(m_inv, ops)}.dot(Vector({[float(v_m[0, i]) for i in range(len(output))]}))",
            f"res = []",
            f"for el in v:",
            f"  res.append(round(el))",
            f"print(bytes(res))",
        ]
    )


class Checker(BaseChecker):
    vulns: int = 1
    timeout: int = 15
    uses_attack_data: bool = True

    def __init__(self, *args, **kwargs):
        super(Checker, self).__init__(*args, **kwargs)
        self.c = CheckMachine(self)

    def action(self, action, *args, **kwargs):
        try:
            super(Checker, self).action(action, *args, **kwargs)
        except requests.exceptions.ConnectionError:
            self.cquit(Status.DOWN, "Connection error", "Got requests connection error")
        except (
            ConnectionRefusedError,
            websocket.WebSocketBadStatusException,
            websocket.WebSocketProtocolException,
        ):
            self.cquit(
                Status.DOWN, "Connection error", "Got websocket connection error"
            )
        except websocket.WebSocketTimeoutException:
            self.cquit(Status.DOWN, "Connection error", "Got websocket timeout error")

    def check_random_string(self):
        session = get_initialized_session()
        username = rnd_username()
        password = rnd_password()
        self.c.register(session, username, password, Status.MUMBLE)
        output = rnd_string(16)
        repl_code = program_with_string_output(output, ops=0)
        repl_id = self.c.put_repl(session, repl_code, Status.MUMBLE)

        ws = self.c.run_repl(session, repl_id, Status.CORRUPT)
        _, data = ws.recv_data()
        self.assert_in(output.encode(), data, "invalid repl with flag", Status.CORRUPT)
        self.cquit(Status.OK)

    def check_struct(self):
        session = get_initialized_session()
        username = rnd_username()
        password = rnd_password()
        self.c.register(session, username, password, Status.MUMBLE)
        output = rnd_string(128)
        array = [
            struct.unpack("d", output.encode()[i : i + 8])[0]
            for i in range(0, len(output), 8)
        ]

        repl_code = "\n".join(
            [
                "from numb import Vector",
                "import struct",
                f"v = Vector({array})",
                f"res = []",
                f"for el in v:",
                f'  res.extend(struct.pack("d", el))',
                f"print(bytes(res))",
            ]
        )
        repl_id = self.c.put_repl(session, repl_code, Status.MUMBLE)

        ws = self.c.run_repl(session, repl_id, Status.CORRUPT)
        _, data = ws.recv_data()
        self.assert_in(output.encode(), data, "invalid repl with flag", Status.CORRUPT)
        self.cquit(Status.OK)

    def check_input(self):
        session = get_initialized_session()
        username = rnd_username()
        password = rnd_password()
        self.c.register(session, username, password, Status.MUMBLE)

        v1 = [random.uniform(-1337, 1337) for _ in range(10)]
        v2 = [random.uniform(-1337, 1337) for _ in range(10)]

        repl_code = "\n".join(
            [
                "from numb import Vector",
                "v1 = Vector([float(input()) for _ in range(10)])",
                "v2 = Vector([float(input()) for _ in range(10)])",
                f"print(v1.dot(v2))",
            ]
        )
        repl_id = self.c.put_repl(session, repl_code, Status.MUMBLE)

        ws = self.c.run_repl(session, repl_id, Status.CORRUPT)
        ws.send_binary("".join(repr(i) + "\n" for i in v1 + v2).encode())
        _, data = ws.recv_data()
        try:
            res = float(data)
            self.assert_eq(
                res, np.dot(v1, v2), "invalid repl with flag", Status.CORRUPT
            )
            self.cquit(Status.OK)
        except ValueError:
            self.cquit(
                Status.MUMBLE,
                "could not convert to float",
                "could not convert to float",
            )

    def check(self):
        choice = random.choice(["string", "input", "struct"])
        if choice == "string":
            self.check_random_string()
        elif choice == "input":
            self.check_input()
        elif choice == "struct":
            self.check_struct()

    def put(self, flag_id: str, flag: str, vuln: str):
        session = get_initialized_session()
        username = rnd_username()
        password = rnd_password()

        self.c.register(session, username, password, Status.MUMBLE)
        repl_code = program_with_string_output(flag, ops=0)
        repl_id = self.c.put_repl(session, repl_code, Status.MUMBLE)
        self.cquit(
            Status.OK,
            json.dumps({"repl_id": repl_id}),
            json.dumps(
                {
                    "username": username,
                    "password": password,
                    "repl_id": repl_id,
                }
            ),
        )

    def get(self, flag_id: str, flag: str, vuln: str):
        flag_data = json.loads(flag_id)
        username = flag_data["username"]
        password = flag_data["password"]
        repl_id = flag_data["repl_id"]
        session = get_initialized_session()
        self.c.login(session, username, password, Status.CORRUPT)
        ws = self.c.run_repl(session, repl_id, Status.CORRUPT)
        _, data = ws.recv_data()
        self.assert_in(flag.encode(), data, "invalid repl with flag", Status.CORRUPT)
        self.cquit(Status.OK)


if __name__ == "__main__":
    c = Checker(sys.argv[2])

    try:
        c.action(sys.argv[1], *sys.argv[3:])
    except c.get_check_finished_exception():
        cquit(Status(c.status), c.public, c.private)
