archlinux=$(type pacman >/dev/null 2>/dev/null; echo $?)
debian=$(type apt >/dev/null 2>/dev/null; echo $?)

installDeps() {
	if [ $debian -eq 0 ] ; then
		sudo apt install xorg libx11-dev cmake ninja-build libgoogle-glog-dev g++
	elif [ $archlinux -eq 0 ] ; then
		sudo pacman -S --needed xorg-server libx11 cmake ninja google-glog gcc
	else
		printf "$0: Distribution could not be identified! Please install the dependencies listed in the README.md file.\n"
		exit 1
	fi
}

printf "$0: Installing dependencies... \n"
installDeps

printf "$0: Looking for build directory... \n"

[ -d ./build ] || mkdir ./build
cd ./build

printf "$0: Generating build files... \n"
cmake .. -G Ninja

printf "$0: Building... \n"
ninja && printf "$0: All done! Built binary is in \`build/\`\n"
cd ..

