#include "Application.h"
#include <memory>

// SDL is needed to handle our main function
#include <SDL.h>

// Include folder required for _CrtSetDbgFlag
#include <crtdbg.h>

int main(int argc, char** argv)
{
	// Detect memory leaks during debugging
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	auto application = std::make_unique<Applicataion>();
	return application->Execute();
}