#!/bin/sh

### This script is run in a new process by the minimal init inside the microVM
### It is used to launch the microVM managing internal programs for the PSAR project with the right parameters

cd /
. venv/bin/activate
export INIT_FUNC="$1"
exec gunicorn -w 1 -b 0.0.0.0 app:app
