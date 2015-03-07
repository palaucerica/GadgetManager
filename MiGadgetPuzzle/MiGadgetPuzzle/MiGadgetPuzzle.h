// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MIGADGETPUZZLE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MIGADGETPUZZLE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MIGADGETPUZZLE_EXPORTS
#define MIGADGETPUZZLE_API __declspec(dllexport)
#else
#define MIGADGETPUZZLE_API __declspec(dllimport)
#endif

// This class is exported from the MiGadgetPuzzle.dll
class MIGADGETPUZZLE_API CMiGadgetPuzzle {
public:
	CMiGadgetPuzzle(void);
	// TODO: add your methods here.
};

extern MIGADGETPUZZLE_API int nMiGadgetPuzzle;

MIGADGETPUZZLE_API int fnMiGadgetPuzzle(void);
