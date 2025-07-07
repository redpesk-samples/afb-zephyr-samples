#!/bin/sh

set -x
set -e
openssl ecparam -out key.pem -name prime256v1 -genkey
openssl req -new -key key.pem -x509 -nodes -days 3650 -out cert.pem
openssl x509 -in cert.pem -outform DER -out cert.der
openssl pkey -in key.pem -outform DER -out key.der
