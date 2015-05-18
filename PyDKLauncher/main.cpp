
#include "stdafx.h"
#include <DK/DK.h>
#include <PyDK/PyDK.h>

using namespace DKFoundation;
using namespace DKFramework;

DKString AbsolutePathStringOfPyStd(void);

#ifdef _WIN32
#pragma comment (lib, "DK.lib")
#pragma comment (lib, "PyDK.lib")

DKString AbsolutePathStringOfPyStd(void)
{
	wchar_t path[MAX_PATH];
	for (DWORD len = ::GetModuleFileNameW(NULL, path, MAX_PATH); len > 0; len--)
	{
		if (path[len - 1] == L'\\')
		{
			path[len] = L'\0';
			return DKString(path).Append("python34.zip");
		}
	}
	return L"";
}
#endif

// inherit PyDKApplication (Not DKApplication directly)
// PyDKApplication enables to use Python-Context.
class DemoApp : public PyDKApplication
{
public:
	DKObject<PyDKPython> python;
	DKObject<DKThread> thread;

	void OnInitialize(void)
	{
		DKString pyStdPath = AbsolutePathStringOfPyStd();
		DKString scriptPath = EnvironmentPath(SystemPathAppResource).Append("/Scripts");

		this->python = PyDKPython::Create({ pyStdPath, scriptPath });
		if (this->python)
		{
			DKLog("Python Lib: %ls\n", (const wchar_t*)pyStdPath);
			DKLog("Script Root: %ls\n", (const wchar_t*)scriptPath);

			DKString mainScript = DKString(scriptPath).Append("/main.py");

			auto runScript = [this](DKString mainScript)
			{
				// run main script. (main.py)
				this->python->RunFile(mainScript);
				
				// script finished. terminate app.
				this->CancelScriptBinding(); // unbind PyDK.
				this->Terminate(0);
			};

			this->thread = DKThread::Create(DKFunction(runScript)->Invocation(mainScript));

			// Call PyDKApplication::OnInitialize() to initialize PyDK.
			return PyDKApplication::OnInitialize();
		}
		Terminate(-1);
	}
	void OnTerminate(void)
	{
		if (this->python)
		{
			PyDKApplication::OnTerminate();
			this->thread->WaitTerminate();
			this->python = NULL;	// destroy python context.
		}
	}
};

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	return DemoApp().Run();
}
