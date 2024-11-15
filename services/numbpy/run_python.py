from RestrictedPython import compile_restricted
from RestrictedPython import safe_globals
from RestrictedPython.PrintCollector import PrintCollector

import numb
import importlib
import struct
from operator import getitem
import sys


def safe_import(module_name: str, globals=None, locals=None, fromlist=(), level=0):
    if module_name in {"numb", "struct"}:
        return importlib.__import__(module_name, globals, locals, fromlist, level)


class Printer:
    def __init__(self):
        pass

    def _call_print(self, *args):
        print(*args)


with open(sys.argv[1]) as f:
    source_code = f.read()

loc = {}
byte_code = compile_restricted(source_code, "<inline>", "exec")

safe_globals["__builtins__"]["__import__"] = safe_import
safe_globals["_getitem_"] = getitem
safe_globals["_getiter_"] = lambda x: x
safe_globals["_write_"] = lambda x: x
safe_globals["_print_"] = lambda *x: Printer()
safe_globals["input"] = input

exec(byte_code, safe_globals, loc)
