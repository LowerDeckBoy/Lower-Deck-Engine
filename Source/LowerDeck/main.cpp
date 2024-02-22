
#include "Application.hpp"

_Use_decl_annotations_
int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	auto* app = new lde::App(hInstance, { 1280, 720, false });

	try
	{
		app->Initialize();
		app->Run();
		app->Release();
	}
	catch (...)
	{

	}

	return 0;
}
