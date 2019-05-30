#! /bin/bash

bash -c "http-server ./ -p 8000" &
bash -c "nodejs camera-live.js" &
bash -c "nodejs config.js" &
#bash -c "python client.py --ip=<ip:port>" &
