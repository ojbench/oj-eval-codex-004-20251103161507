#!/bin/bash
rm -f *.dat

cat << 'INPUT' | ./code
su root sjtu
useradd worker0 worker0_password 3 Palmer,SnyderandClark
passwd root sjtuuu
logout
su worker0 worker0_password
useradd user0 user0_password 1 Huynh,CardenasandGarcia
register user1 user1_password Miller-Sandoval
logout
su root sjtuuu
delete worker0
logout
su user0 user0_password
passwd user1 user1_password user1_passworddd
exit
INPUT
