#!/bin/bash

make

dir="./samples"

green=$(tput setaf 2)
red=$(tput setaf 1)
reset=$(tput sgr0)

for file in $dir/*.decaf; do
  base=$(basename $file .decaf)
  ./dcc < $file >& $dir/$base.tmp
  diff $dir/$base.tmp $dir/$base.out

  if [ $? -eq 0 ]; then
    echo "${green}$base: Output matches expected result${reset}"
  else
    echo "${red}$base: ERROR: Output does not match expected result${reset}"
  fi
  rm $dir/$base.tmp
done
