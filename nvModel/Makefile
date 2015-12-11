HEADERS=nvModel.h nvMath.h nvMatrix.h nvQuaternion.h nvSDKPath.h \
        nvShaderUtils.h nvUtils.h nvVector.h

SRC=nvModel.cc nvUtils.cc nvUtils.cc nvModelObj.cc nvModelQuery.cc

obj/nvModel:$(HEADERS) $(SRC)
	g++ -c $(SRC)
	mv *.o obj/

clean:
	rm -f obj/*.o
