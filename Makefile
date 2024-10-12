
flags = -O1 # $(flags)
libraries = -I/usr/include/python3.10 -lpython3.10 # $(libraries)

hanoi:
	g++ -O3 bf.cpp $(libraries)
	./a.out benches/hanoi.b   $(flags)
	@echo "linking.."
	gcc bf.s 
	time ./a.out

mandel:
	g++ -O3 bf.cpp $(libraries)
	./a.out benches/mandel.b $(flags)
	@echo "linking.."
	gcc bf.s 
	time ./a.out
	

serptri:
	g++ -O3 bf.cpp 
	./a.out benches/serptri.b $(flags)
	@echo "linking.."
	gcc bf.s 
	./a.out

hello:
	g++ -O3 bf.cpp -o jbf
	./jbf benches/hello.b -O1
	@echo "linking.."
	gcc bf.s 
	./a.out

test:
	g++ -O3 bf.cpp -o jbf

	./jbf benches/bench.b $(flags)
	gcc bf.s 
	./a.out

	./jbf benches/bottles.b $(flags)
	gcc bf.s 
	./a.out

	./jbf benches/deadcodetest.b $(flags)
	gcc bf.s 
	./a.out

	./jbf benches/hanoi.b $(flags)
	gcc bf.s 
	./a.out

	./jbf benches/hello.b $(flags)
	gcc bf.s 
	./a.out
	
	./jbf benches/long.b $(flags)
	gcc bf.s
	@echo -------------starting long.b test, dont panic---------
	./a.out

	./jbf benches/loopremove.b $(flags)
	gcc bf.s 
	./a.out
	
	./jbf benches/mandel.b $(flags)
	gcc bf.s 
	./a.out

	./jbf benches/serptri.b $(flags)
	gcc bf.s 
	./a.out
	
	./jbf benches/twinkle.b $(flags)
	gcc bf.s 
	./a.out	
	@echo "All tests completed."


	
twinkle:
	g++ -O3 bf.cpp -o jbf
	./jbf benches/twinkle.b
	gcc bf.s
	./a.out
	
check:
	g++ -O3 bf.cpp -o jbf  $(libraries)
	./jbf check.b $(flags)  
	gcc bf.s 
	./a.out <input.dat


compile:
	g++ -g bf.cpp


showcase:
	time ../a5/a.out benches/mandel.b 
	g++ -O3 bf.cpp 
	./a.out benches/mandel.b 
	@echo "linking.."
	gcc bf.s 
	time ./a.out
	g++ -O3 bf.cpp 
	sleep 3
	./a.out benches/mandel.b $(flags)
	@echo "linking.."
	gcc bf.s 
	time ./a.out

py:
	python3 test.py
