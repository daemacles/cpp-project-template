#!/bin/bash

# Removes links to the template git repository and creates this as a new one.

echo "Do you wish to break ties with the cpp-project-template repo and"
echo "create a brand new one in its place?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) exit;;
    esac
done

rm -rf .git
git init .
git add CMakeLists.txt src/main.cc .gitignore .ycm_extra_conf.py LICENSE \
        README.md include compile_cycle.sh
git commit -m "Initial commit."

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
cd ..
rm boot.sh

ctags-exuberant --recurse=yes

echo
echo "Initial git commit has occured and project has been built in the build/"
echo "directory."
