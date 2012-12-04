#!/bin/bash

xmlrpc http://localhost:5440/RPC2 ds50.stop &
xmlrpc http://localhost:5441/RPC2 ds50.stop &

wait

xmlrpc http://localhost:5450/RPC2 ds50.stop &
xmlrpc http://localhost:5451/RPC2 ds50.stop &

wait
