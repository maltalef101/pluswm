isInstalled() {
	distro=$(whatDistro)

	packagesNotInstalled=""

	if [ $distro = "debian" ]; then
		for i in "$1"; do
			[ apt list | grep "^$i.*installed" ] || printf "$0: $i is not installed!\n"; $packagesNotInstalled=$packagesNotInstalled"$i "
		done
		apt install $packagesNotInstalled
	fi
}

whatDistro() {
	osRelease="$(cat /etc/os-release | awk -F= '/^NAME/ {print $2}')"

	if [ $osRelease = "Debian GNU/Linux" ]; then
		return "debian"
	elif [ $osRelease = "Arch Linux" ]; then
		return "arch"
	else
		return "unknown"
	fi
}

installDeps() {
	distro=$(whatDistro)
	if [ $distro = "debian" ]; then
		isInstalled "xorg libx11-dev cmake ninja-build libgoogle-glog-dev g++"
	elif [ $distro = "arch" ]; then
		isInstalled "xorg libx11 cmake ninja google-glog gcc"
	else
		printf "$0: Distribution could not be identified! Please install the dependencies listed in the README.md file."
		exit 1
	fi
}

printf "$0: Checking for dependencies... \n"
installDeps

printf "$0: Looking for build directory... \n"

[ -d ./build ] || mkdir ./build
cd ./build

printf "$0: Generating build files... \n"
cmake .. -G Ninja

printf "$0: Building... \n"
ninja && printf "$0: All done! Built binary is in \`build/\`\n"
cd ..

