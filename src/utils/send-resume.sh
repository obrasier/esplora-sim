#!/bin/sh
$(cd $(dirname $0); pwd)/send-client-event.sh '[ { "type": "resume", "data": { "resume": '1'} } ]'
