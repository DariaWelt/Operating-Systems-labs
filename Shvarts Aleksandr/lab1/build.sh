#!/bin/bash
b_dir="build"
prog_name="watchdog"
log_path="/var/log/my${prog_name}.log"

mkdir ${b_dir}
cd ${b_dir} || exit

cmake ../
cmake --build ./
mv $prog_name ../

cd ../
rm -rf ${b_dir:?}/

# create config
echo "local0.* -${log_path}" > my.conf
sudo mv my.conf /etc/rsyslog.d
# add the local0 rule to the rsyslog
#sudo echo "local0.* -${log_path}" | sudo tee -a /etc/rsyslog.conf
# update rsyslog
sudo systemctl restart rsyslog
