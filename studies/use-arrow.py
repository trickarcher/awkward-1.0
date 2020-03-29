import pyarrow as pa
import pandas as pd
import numpy as np
from ctypes import *

''' 
Create ArrowArray and ArrowSchema structs which 
will essentially link to the C++ Library libfoo.so
'''
class ArrowSchema(Structure):
    pass

# The release function pointer
release_func_ptr_schema = CFUNCTYPE(c_void_p, POINTER(ArrowSchema))

ArrowSchema._fields_ = [("format", c_char_p),
                        ("name", c_char_p),
                        ("metadata", c_char_p),
                        ("flags", c_longlong),
                        ("n_children", c_longlong),
                        ("children", POINTER(POINTER(ArrowSchema))),
                        ("dictionary", POINTER(ArrowSchema)),
                        ("release", release_func_ptr_schema),
                        ("private_data", c_void_p)]


class ArrowArray(Structure):
    pass

# The release function pointer
release_func_ptr_array = CFUNCTYPE(c_void_p, POINTER(ArrowArray))

ArrowArray._fields_ =[("length", c_longlong),
                      ("null_count", c_longlong),
                      ("offset", c_longlong),
                      ("n_buffers", c_longlong),
                      ("n_children", c_longlong),
                      ("buffers", POINTER(c_void_p)),
                      ("children", POINTER(POINTER(ArrowArray))),
                      ("dictionary", POINTER(ArrowArray)),
                      ("release", release_func_ptr_array),
                      ("private_data", c_void_p)]

# Load the C++ library
lib = cdll.LoadLibrary('./libfoo.so')

# Initialize a pyarrow Array!
batch = pa.RecordBatch.from_arrays([pa.array([0, 1, 2, 3, 4, 5])], ["one"])
sink = pa.BufferOutputStream()

writer = pa.RecordBatchStreamWriter(sink, batch.schema)

writer.write_batch(batch)
writer.close()
buf = sink.getvalue()
print(np.frombuffer(buf, dtype=np.uint8)[272:].tostring())


# Create a int32_t array
int64_arr = c_longlong * 6
arr_ptr = int64_arr(0, 1, 2, 3, 4, 5)

scheme = ArrowSchema()
lib.export_int64_type(pointer(scheme))

# Initialize a ArrowArray struct
arrow_arr = ArrowArray()

# Call the C++ library function to convert the int32_t array into a ArrowArray
lib.export_int64_array(arr_ptr, 10, pointer(arrow_arr))

# The bytes get printed! So the above operation was a success. 
# You can validate by accessing members of struct
print(np.frombuffer(arrow_arr, dtype=np.uint8).tostring())