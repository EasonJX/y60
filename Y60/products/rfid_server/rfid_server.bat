#! /bin/bash

rfid_serverOPT 127.0.0.1 --saturation 1000 --halflife 2 --power 30 --hs --expected 3 --prefix "name: " --remoteport 2343 
