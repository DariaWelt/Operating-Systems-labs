pidPath="/var/run/yupichkin_lab1.pid"

[[ -f pidPath ]] || sudo touch "$pidPath"

sudo chmod 666 "$pidPath"

mkdir build
# shellcheck disable=SC2164
cd build
cmake -S ../ -B ./
make
cp ./daemon ../daemon
cd ../
rm -r build/