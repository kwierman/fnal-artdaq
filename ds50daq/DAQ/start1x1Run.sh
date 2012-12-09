#!/bin/bash

runNumber=0101

xmlrpc http://localhost:5450/RPC2 ds50.start $runNumber

xmlrpc http://localhost:5440/RPC2 ds50.start $runNumber
