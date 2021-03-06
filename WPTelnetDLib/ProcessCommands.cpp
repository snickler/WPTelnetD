#include "pch.h"
#include "ProcessCommands.h"

void KillCommand::ProcessCommand(Connection *pConnection, ParsedCommandLine *pCmdLine) {

	if (pCmdLine->GetArgs().size()<2)
		pConnection->WriteLine("SYNTAX: kill PID [exit code]");
	else {
		int num = atoi(pCmdLine->GetArgs().at(1).c_str());
		int exitCode = pCmdLine->GetArgs().size() > 2 ? atoi(pCmdLine->GetArgs().at(2).c_str()) : 0;
		HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, num);


		if (process == (HANDLE)-1) {
			pConnection->WriteLastError();
			return;
		}

		if (!TerminateProcess(process, exitCode))
			pConnection->WriteLastError();
	}
}

string KillCommand::GetName() {
	return "kill";
}

void PsCommand::ProcessCommand(Connection *pConnection, ParsedCommandLine *pCmdLine) {
	HANDLE _handle = ::CreateToolhelp32Snapshot(0x2, 0);


	if (_handle == 0 || _handle == INVALID_HANDLE_VALUE)
	{
		pConnection->WriteLine("ERROR: Failed to create snapshot");
	}
	else {
		PROCESSENTRY32W _process;
		_process.dwSize = sizeof(_process);
		DWORD old = _process.dwSize;
		bool result = Process32FirstW(_handle, &_process);
		
		MEMORYSTATUSEX memStatus = { sizeof(MEMORYSTATUSEX) };
		
		if (GlobalMemoryStatusEx(&memStatus)){
			pConnection->WriteLine("Physical memory available %dmb out of %dmb (%d%% load).", (DWORD)(memStatus.ullAvailPhys / (1024 * 1024)), (DWORD)(memStatus.ullTotalPhys / (1024 * 1024)), memStatus.dwMemoryLoad);
			pConnection->WriteLine("Virtual memory available %dmb out of %dmb.", (DWORD)(memStatus.ullAvailVirtual / (1024 * 1024)), (DWORD)(memStatus.ullTotalVirtual / (1024 * 1024)));
			pConnection->WriteLine("Pagefile available %dmb out of %dmb.", (DWORD)(memStatus.ullAvailPageFile / (1024 * 1024)), (DWORD)(memStatus.ullTotalPageFile / (1024 * 1024)));
			pConnection->WriteLine("");
				
		}
		else
			pConnection->WriteLastError();

		if (!result) {
			pConnection->WriteLine("ERROR: Got false from Process32First: %d %d %d %d %d", GetLastError(), _handle, sizeof(_process), _process.dwSize, old);
		}
		else {


			pConnection->WriteLine("************");
			pConnection->WriteLine("Process List");
			pConnection->WriteLine("************");
			pConnection->WriteLine("");
			char exeFile[1024];
			size_t outnum;
			wcstombs_s(&outnum, exeFile, _process.szExeFile, sizeof(exeFile));
			pConnection->WriteLine("%s (%d,%d,%d)", exeFile, _process.th32ProcessID, _process.th32ParentProcessID, _process.th32ModuleID);

			bool result = true;
			while (result) {
				result = Process32NextW(_handle, &_process);
				if (result) {
					wcstombs_s(&outnum, exeFile, _process.szExeFile, sizeof(exeFile));
					pConnection->WriteLine("%s (%d,%d,%d)", exeFile, _process.th32ProcessID, _process.th32ParentProcessID, _process.th32ModuleID);
				}
			}
			pConnection->WriteLine("");
		}

	}
}

string PsCommand::GetName() {
	return "ps";
}


void WhoAmICommand::ProcessCommand(Connection *pConnection, ParsedCommandLine *pCmdLine) {

	if (pCmdLine->GetArgs().size()<1)
		pConnection->WriteLine("SYNTAX: whoami");
	else {
		HMODULE lib = LoadLibraryA("SSPICLI.dll");
		if (lib == NULL){
			pConnection->WriteLine("Failed Load Lib");
			pConnection->WriteLastError();
			return;
		}

		PGetUserNameExA GetUserNameExA = (PGetUserNameExA)GetProcAddress(lib, "GetUserNameExA");
		if (GetUserNameExA == NULL)
		{
			pConnection->WriteLine("Failed to locate GetUserNameExA");
			return;
		}

		char buf[1024];
		ULONG size = sizeof(buf);
		if (!GetUserNameExA(NameSamCompatible, buf, &size)) {
			pConnection->WriteLastError();
		}
		else
			pConnection->WriteLine("You are '%s'", buf);


	}
}

string WhoAmICommand::GetName() {
	return "whoami";
}