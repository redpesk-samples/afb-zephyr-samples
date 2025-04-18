#!/bin/sh

set -x
openssl req -x509 -days 3000 -newkey rsa:4096 -noenc -keyout key.pem -out cert.pem
openssl x509 -in cert.pem -outform DER -out cert.der
openssl rsa -in key.pem -outform DER -out key.der
