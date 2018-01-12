#/bin/bash
dir="$(dirname "$0")"
source "$dir/.config"
#echo -n "$USER@$HOST's password: "
#read -s PASSWD
echo
sftp $USER@$HOST <<END_SCRIPT
#quote user $USER 
#quote pass $PASSWD
cd $DIR
put index.html
put slimetrail.php
put slimetrail
put slimetrail.css
quit
END_SCRIPT

exit 0
