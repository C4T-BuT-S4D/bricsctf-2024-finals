#!/usr/bin/env python3

import random
import re
import sys
import requests
import checklib
from qrlib import CheckMachine
import segno
import tempfile

class Checker(checklib.BaseChecker):
    vulns: int = 1
    timeout: int = 20
    uses_attack_data: bool = True
    external_web_storage = "http://kek.spok.live:5555"
    

    def __init__(self, *args, **kwargs):
        super(Checker, self).__init__(*args, **kwargs)
        self.c = CheckMachine(self)
        self.uuid_regexp = re.compile(r'^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$')

    def validate_uuid(self, uuid):
        self.assert_eq(bool(self.uuid_regexp.fullmatch(uuid)), True, 'Invalid uuid returned')

    def put_in_external_web_storage(self, token: str, key: str, value: str):
        sess = checklib.get_initialized_session()
        resp = sess.get(f"{self.external_web_storage}/put", params={"token": token, "key": key, "value": value})
        self.check_response(resp, 'Internal error', status=checklib.Status.ERROR)
        
    def action(self, action, *args, **kwargs):
        try:
            super(Checker, self).action(action, *args, **kwargs)
        except requests.exceptions.ConnectionError:
            self.cquit(checklib.Status.DOWN, 'Connection error', 'Got requests connection error')

    def check(self):
        sess = checklib.get_initialized_session()
        username, password = checklib.rnd_username(), checklib.rnd_password()
        self.c.signup(sess, username, password)
        self.c.signin(sess, username, password)

        token = self.c.add_token(sess, "token_" + checklib.rnd_string(2)).get('token')

        sess = checklib.get_initialized_session()

        if random.randint(0, 5) <= 3:
            self.decode_text(sess, token)
        else:
            self.decode_url(sess, token)


        self.cquit(checklib.Status.OK)
    
    def decode_text(self, sess, token):
        for _ in range(10):
            example_text = checklib.rnd_string(32)
            decode_res = self.try_generate_and_upload(sess, token, example_text)
            if decode_res is not None:
                break
        else:
            self.cquit(checklib.Status.MUMBLE, 'Failed to decode qr code')

        self.assert_eq(decode_res.get('content'), example_text, 'Content mismatch')
        self.validate_uuid(decode_res.get('id'))
        
        results = self.c.results(sess, token).get('results', [])
        self.assert_in(example_text, [r.get('content') for r in results], 'Content not found in results')
    
    def decode_url(self, sess, token):
        content = checklib.rnd_string(32)
        
        for _ in range(10):
            key = checklib.rnd_string(15)
            self.put_in_external_web_storage("stupidTokenForChecker", key, content)
            decode_res = self.try_generate_and_upload(sess, token, f"{self.external_web_storage}/{key}", parse=True)
            if decode_res is not None:
                break
        else:
            self.cquit(checklib.Status.MUMBLE, 'Failed to decode qr code')

        self.assert_eq(decode_res.get('content'), content, 'Content mismatch')

        results = self.c.results(sess, token).get('results', [])
        self.assert_in(content, [r.get('content') for r in results], 'Content not found in results')

        
    def put(self, flag_id: str, flag: str, vuln: int):
        sess = checklib.get_initialized_session()
        username, password = checklib.rnd_username(), checklib.rnd_password()
        self.c.signup(sess, username, password)
        uid = self.c.signin(sess, username, password).get('uid')

        token = self.c.add_token(sess, "token_" + checklib.rnd_string(3)).get('token')

        sess = checklib.get_initialized_session()
        for attempt in range(10):
            content = flag + '\n' + ' ' * attempt 
            decode_res = self.try_generate_and_upload(sess, token, content)
            if decode_res is not None:
                break
        else:
            self.cquit(checklib.Status.MUMBLE, 'Failed to decode qr code')
        
        qr_id = decode_res.get('id')
        self.assert_eq(decode_res.get('content').strip(), flag, 'Content mismatch')
        self.validate_uuid(qr_id)

        self.cquit(checklib.Status.OK, f"{username}:{uid}:{qr_id}", f"{username}:{qr_id}:{password}")

    def get(self, flag_id: str, flag: str, vuln: int):
        username, qr_id, password = flag_id.split(':')
        sess = checklib.get_initialized_session()
        self.c.signin(sess, username, password)

        tokens = self.c.tokens(sess, status=checklib.Status.CORRUPT)
        self.assert_gt(len(tokens), 0, 'No tokens found', status=checklib.Status.CORRUPT)

        token = tokens[0].get('token')

        results = self.c.results(sess, token)
        
        self.assert_in(flag, [r.get('content').strip() for r in results.get('results', [])], 'Content not found in results')

        self.cquit(checklib.Status.OK)
    
    
    def try_generate_and_upload(self, sess: requests.Session, token:str, text: str, parse: bool = False):
        with tempfile.NamedTemporaryFile(suffix=".png") as f:
            segno.make(text).save(f.name, scale=5, border=4)
            resp = self.c.try_decode(sess, token, f.name, parse=parse)
            if resp.status_code != 200:
                return None
            return self.get_json(resp, 'Failed to decode qr code')


if __name__ == '__main__':
    c = Checker(sys.argv[2])
    
    try:
        c.action(sys.argv[1], *sys.argv[3:])
    except c.get_check_finished_exception():
        checklib.cquit(checklib.Status(c.status), c.public, c.private)


