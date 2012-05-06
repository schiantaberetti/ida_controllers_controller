#!/bin/sh

ida_info="`/usr/sbin/ida_controller -d0 -f /dev/ida0`"
maila="admin@somehost.com" # TO CHANGE

echo $ida_info
if [ `echo $ida_info | grep -c "OK"` -lt 1 ]; then
	echo "Fault found, sending email... " `date` " " $ida_info >> /var/spool/cron/ida_report
	echo  "Gandalf raid fault detected!!" $ida_info | /usr/local/sbin/sendmail $maila
fi

