from checklib import *
import requests

PORT = 3000

class CheckMachine:
    @property
    def url(self):
        return f"http://{self.c.host}:{self.port}"

    def __init__(self, c: BaseChecker):
        self.c = c
        self.port = PORT

    def signup(self, s: requests.Session, username: str, password: str, status: int = Status.MUMBLE) -> None:
        resp = s.post(f"{self.url}/api/signup", json={"username": username, "password": password})
        self.c.check_response(resp, 'Signup failed', status=status)
    
    def signin(self, s: requests.Session, username: str, password: str, status: int = Status.MUMBLE) -> None:
        resp = s.post(f"{self.url}/api/signin", json={"username": username, "password": password})
        self.c.check_response(resp, 'Signin failed', status=status)
        return self.c.get_json(resp, 'Signin failed')

    def tokens(self, s: requests.Session, status: int = Status.MUMBLE) -> list[dict]:
        resp = s.get(f"{self.url}/api/tokens")
        self.c.check_response(resp, 'Tokens failed', status=status)
        return self.c.get_json(resp, 'Tokens failed')['tokens']
    
    def add_token(self, s: requests.Session, name: str, status: int = Status.MUMBLE) -> str:
        resp = s.post(f"{self.url}/api/tokens", json={"name": name})
        self.c.check_response(resp, 'Add token failed', status=status)
        return self.c.get_json(resp, 'Add token failed')['token']
    
    def delete_token(self, s: requests.Session, token: str, status: int = Status.MUMBLE) -> None:
        resp = s.delete(f"{self.url}/api/tokens", json={"token": token})
        self.c.check_response(resp, 'Delete token failed', status=status)
    
    def try_decode(self, s: requests.Session, token: str, qr_path: str, status: int = Status.MUMBLE, parse: bool = False) -> requests.Response:
        if parse:
            params = {"parse": "true"}
        else:
            params = None
        return s.post(f"{self.url}/api/decode", 
                      headers={"Authorization": f"Bearer {token}"}, 
                      files={"qr": open(qr_path, "rb")}, params=params)
    
    def decode(self, s: requests.Session, token: str, qr_path: str, status: int = Status.MUMBLE) -> dict:
        resp = s.post(f"{self.url}/api/decode", 
                      headers={"Authorization": f"Bearer {token}"}, 
                      files={"qr": open(qr_path, "rb")})
        self.c.check_response(resp, 'Decode failed', status=status)
        return self.c.get_json(resp, 'Decode failed')
    
    def results(self, s: requests.Session, token: str, status: int = Status.MUMBLE) -> dict:
        resp = s.get(f"{self.url}/api/results", 
                     headers={"Authorization": f"Bearer {token}"}
                     )
        self.c.check_response(resp, 'Results failed', status=status)
        return self.c.get_json(resp, 'Results failed')
