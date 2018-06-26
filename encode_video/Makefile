EXEC=$(shell basename $(PWD))
SRC=$(shell ls *.c)
LIBS=-lavutil -lavformat -lavcodec -lswscale -lz -lm 

$(EXEC):$(SRC)
	gcc -fdiagnostics-color=auto -g -o $(EXEC) $(SRC) $(LIBS)

run:$(EXEC)
	./$(EXEC) ./raw_nv12_1920x1080.yuv

clean:
	rm -rf $(EXEC)
