from __future__ import print_function

from os.path import dirname, abspath, join
import sys
import zmq
import json
import time
import signal
import sys

# handle ctrl+c to disconnect
def signal_handler(sig, frame):
    print('You pressed Ctrl+C! -> closing ')
    context.destroy()
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

print('connecting...')
context = zmq.Context()
sub=context.socket(zmq.SUB)
sub.setsockopt(zmq.SUBSCRIBE, b"")
sub.RCVTIMEO = 5000 # wait only 5s for new message 
sub.connect('tcp://10.42.2.29:5555')  # Note
print('connected')

print('receiving')
while True:
    j = sub.recv_json()
    if j['frame_number'] % 150 == 0:
        print(str(j['frame_number']) + ' \ ' + str(j['my_frame_number']) + ' on since ' + str(j['on_time']) + ' with time stamp' + str(j['time_stamp']))
        #print('closed: ' + str(sub.closed))
        print(str(j['subject_2']['global_translation']))
    print(json.dumps(j, indent=4, sort_keys=True))