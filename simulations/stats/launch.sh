echo 'RUNNING graph test common'
./run ScenarioTest/omnetpp.ini -u Cmdenv | grep "#___#" > stats/test.dat
cd stats
echo 'Generating graphs'
cat test.dat | grep 'User_A' | sed 's/.*User_A //g' > test_a.dat
cat test.dat | grep 'User_B' | sed 's/.*User_B //g'> test_b.dat
cat test.dat | grep 'User_C' | sed 's/.*User_C //g'> test_c.dat
cat test.dat | grep 'User_D' | sed 's/.*User_D //g'> test_d.dat

gnuplot generate.gnu
cd ..
echo 'END!'

