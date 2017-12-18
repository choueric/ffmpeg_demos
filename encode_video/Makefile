EXEC=$(shell basename $(PWD))

SRC=$(shell ls *.c)

LIBS=-lavutil -lavformat -lavcodec -lswscale -lz -lm 

$(EXEC):$(SRC)
	gcc -fdiagnostics-color=auto -g -o $(EXEC) $(SRC) $(LIBS)

run:$(EXEC)
	./$(EXEC) ~/test_tools/mwcodec/example/raw_i420_1920x1080.yuv

clean:
	rm -rf $(EXEC) sep_hevc/*
