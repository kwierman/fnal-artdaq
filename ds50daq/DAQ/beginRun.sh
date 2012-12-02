#!/bin/bash

runNumber=101

xmlrpc http://localhost:5450/RPC2 ds50.start $runNumber
xmlrpc http://localhost:5451/RPC2 ds50.start $runNumber

xmlrpc http://localhost:5440/RPC2 ds50.start $runNumber
xmlrpc http://localhost:5441/RPC2 ds50.start $runNumber
