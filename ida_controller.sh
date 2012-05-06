#!/bin/sh

ida_info="`ida_controller -d0 -f /dev/ida0`"
maila="luca.baldesi@stud.unifi.it"

if [ `echo $ida_info | grep -c "OK"` -lt 1 ]; then
	echo "Fault found, sending email...."
	echo  "Gandalf raid fault detected!!" $ida_info | /usr/local/sbin/sendmail $maila
fi
