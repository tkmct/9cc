9cc: 9cc.c

test: 9cc
	bash test.sh

clean:
	rm -f 9cc *.o *~ tmp*
