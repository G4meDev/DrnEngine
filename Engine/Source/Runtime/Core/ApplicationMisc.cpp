#include "DrnPCH.h"
#include "ApplicationMisc.h"

namespace Drn
{
	void ApplicationMisc::ClipboardCopy(const std::wstring& Str)
	{
		if( OpenClipboard(GetActiveWindow()) )
		{
			drn_check(EmptyClipboard());
			HGLOBAL GlobalMem;
			int32 StrLen = Str.size();
			GlobalMem = GlobalAlloc( GMEM_MOVEABLE, sizeof(wchar_t)*(StrLen+1) );
			drn_check(GlobalMem);
			wchar_t* Data = (wchar_t*) GlobalLock( GlobalMem );
			wcscpy(Data, Str.c_str());
			//std::wcscpy()
			//strcpy(Data, Str.c_str());
			GlobalUnlock( GlobalMem );
			if( SetClipboardData( CF_UNICODETEXT, GlobalMem ) == NULL )
				//UE_LOG(LogWindows, Fatal,TEXT("SetClipboardData failed with error code %i"), (uint32)GetLastError() );
				std::cout << "failed to copy to clipboard!\n";
			drn_check(CloseClipboard());
		}
	}

	void ApplicationMisc::ClipboardPaste(std::wstring& Result)
	{
		if( OpenClipboard(GetActiveWindow()) )
		{
			HGLOBAL GlobalMem = NULL;
			bool Unicode = 0;
			GlobalMem = GetClipboardData( CF_UNICODETEXT );
			Unicode = 1;
			if( !GlobalMem )
			{
				GlobalMem = GetClipboardData( CF_TEXT );
				Unicode = 0;
			}
			if( !GlobalMem )
			{
				Result = L"";
			}
			else
			{
				void* Data = GlobalLock( GlobalMem );
				drn_check( Data );	
				if( Unicode )
					Result = (wchar_t*) Data;
				else
				{
					char* ACh = (char*) Data;
					int32 i;
					for( i=0; ACh[i]; i++ );
					std::vector<char> Ch;
					Ch.resize(i+1);
					for( i=0; i<Ch.size(); i++ )
						Ch[i]=static_cast<char>(ACh[i]);
					
					std::string ResStr(Ch.data());
					Result = StringHelper::s2ws(ResStr);
					//Result.GetCharArray() = MoveTemp(Ch);
				}
				GlobalUnlock( GlobalMem );
			}
			drn_check(CloseClipboard());
		}
		else 
		{
			Result = L"";
		}
	}

        }