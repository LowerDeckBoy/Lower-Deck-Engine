
#include "Application.hpp"

_Use_decl_annotations_
int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	auto* app = new lde::App(hInstance, { 1600, 900, false });

	try
	{
		app->Initialize();
		app->Run();
		app->Release();
		delete app;
	}
	catch (...)
	{
		delete app;
	}

	return 0;
}
