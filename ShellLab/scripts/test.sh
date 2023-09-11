set -ex

if ! [ -d output ]; then mkdir output; fi

make test01 > output/test01
make test02 > output/test02
make test03 > output/test03
make test04 > output/test04
make test05 > output/test05
make test06 > output/test06
make test07 > output/test07
make test08 > output/test08
make test09 > output/test09
make test10 > output/test10
make test11 > output/test11
make test12 > output/test12
make test13 > output/test13
make test14 > output/test14
make test15 > output/test15
make test16 > output/test16

make rtest01 > output/rtest01
make rtest02 > output/rtest02
make rtest03 > output/rtest03
make rtest04 > output/rtest04
make rtest05 > output/rtest05
make rtest06 > output/rtest06
make rtest07 > output/rtest07
make rtest08 > output/rtest08
make rtest09 > output/rtest09
make rtest10 > output/rtest10
make rtest11 > output/rtest11
make rtest12 > output/rtest12
make rtest13 > output/rtest13
make rtest14 > output/rtest14
make rtest15 > output/rtest15
make rtest16 > output/rtest16