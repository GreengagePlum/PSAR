"""
This is the Flask web server that is to run inside the microVM
to execute user functions and forward results

It executes user functions (bundled as standalone executables)
upon an HTTP GET request to the root (/) endpoint and returns
what the function writes to its stdout as json or an error if
any occured during this procedure.

It is launched by the minimal init inside the microVM and can
be made to change which function it executes by being instructed
via a named POSIX FIFO.
"""

import os
import pickle
import subprocess
from flask import Flask
from flask import jsonify

FIFO_path = "/tmp/SERV_TO_VSOCKMAN"

# Open fifo for writing
# print("Opening FIFO...")
# serv_to_vsock = open(FIFO_path, mode="wb", buffering=0)

app = Flask(__name__)
function = os.getenv("INIT_FUNC")  # Sets the initial function to execute


@app.route("/")
def func_endpoint():
    try:
        ret = subprocess.run(
            function, capture_output=True, timeout=None, check=True, encoding="utf-8"
        )
    except subprocess.CalledProcessError as e:
        return (
            f"Executed function encountered an error: returncode={e.returncode}",
            500,
        )
    except subprocess.TimeoutExpired as e:
        return (
            f"Executed function was timed out: timeout={e.timeout} (in seconds)",
            500,
        )

    return jsonify(ret.stdout)


if __name__ == "__main__":
    app.run(debug=True)
