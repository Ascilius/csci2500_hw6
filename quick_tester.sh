#!/bin/bash

printf "\nTesting \"example1.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example1.src

printf "\nTesting \"example2.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example2.src

printf "\nTesting \"example3.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example3.src

printf "\nTesting \"example4.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example4.src

printf "\nTesting \"example5.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example5.src

printf "\nTesting \"test05.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/test05.txt

printf "\nTesting \"example6.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example6.txt

printf "\nTesting \"example7.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example7.txt

printf "\nTesting \"example8.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example8.txt

printf "\nTesting \"example9.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example9.txt

printf "\nTesting \"example10.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example10.txt

printf "\nTesting \"example11.txt\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example11.txt

printf "\nTesting \"example12.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example12.src

printf "\nTesting \"example13.src\"...\n\n"
valgrind --leak-check=full ./build/hw6 tests/example13.src