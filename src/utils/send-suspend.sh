#!/bin/sh
$(cd $(dirname $0); pwd)/send-client-event.sh '[ { "type": "suspend", "data": { "suspend": '1'} } ]'
