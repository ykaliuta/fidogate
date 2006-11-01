#
# Regular cron jobs for the fidogate package
#
*/10 *	* * *	ftn	/usr/bin/runinc
*/10 *	* * *	news	/usr/bin/send-fidogate
*/10 *	* * *	ftn	/usr/bin/runinc -o

