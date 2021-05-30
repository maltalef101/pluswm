printf "$0: Looking for build directory... \n"

[ -d ./build ] || mkdir ./build
cd ./build

printf "$0: Generating build files... \n"
cmake .. -G Ninja

printf "$0: Building... \n"
ninja && printf "$0: All done! Built binary is in \`build/\`\n"
cd ..
