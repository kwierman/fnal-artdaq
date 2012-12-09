#!/bin/bash

xmlrpc http://localhost:5440/RPC2 ds50.stop &
xmlrpc http://localhost:5441/RPC2 ds50.stop &
xmlrpc http://localhost:5442/RPC2 ds50.stop &
xmlrpc http://localhost:5443/RPC2 ds50.stop &

wait

xmlrpc http://localhost:5450/RPC2 ds50.stop &
xmlrpc http://localhost:5451/RPC2 ds50.stop &
xmlrpc http://localhost:5452/RPC2 ds50.stop &
xmlrpc http://localhost:5453/RPC2 ds50.stop &
xmlrpc http://localhost:5454/RPC2 ds50.stop &

wait
