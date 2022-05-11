//! Function description in CommandFuncBox::CommandFuncBox()

namespace cmdFuncs {
	
	int8_t  ver(int argc, char* argv[]) {
		sendf(F(CMDLOOPLIBVERSION));
		return 0;
	}
	
	int8_t printArgs(int argc, char* argv[]) {
		for (uint8_t i = 0; i < argc; i++)
			sendf(F("%d: >%s<\n"),i,argv[i]);
		return 0;
	}

};
