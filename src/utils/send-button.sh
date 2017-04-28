#!/bin/sh
$(cd $(dirname $0); pwd)/send-client-event.sh '[ { "type": "arduino_mux", "data": { "pin": '$1', "voltage": '$2' } } ]'
