#include "DrnPCH.h"
#include "D3D12Utils.h"

//RENDERDOC_API_1_1_2* rdoc_api = NULL;
//bool bCapturingFrame = false;

//void VerifyD3D12Result(HRESULT Result, const char* Code, const char* Filename, UINT Line)
//{
//	std::cout << std::sprintf("d3d12 failed at %s %s %s", Filename, Line, Code);
//}

static std::string GetUniqueName()
{
	static std::atomic<int64> ID = 0;
	const int64 UniqueID = ID.fetch_add(1);
	const std::string UniqueName = std::string("D3D12Resource_") + std::to_string(UniqueID);
	return UniqueName;
}


void SetName(ID3D12Object* const Object, const std::string& Name)
{
#if D3D12_Debug_INFO
	if (Object && !Name.empty())
	{
		Object->SetName(Drn::StringHelper::s2ws(Name).c_str());
	}
	else if (Object)
	{
		Object->SetName(Drn::StringHelper::s2ws(GetUniqueName()).c_str());
	}
#endif
}

void SetName(Drn::RenderResource* const Resource, const std::string& Name)
{
#if D3D12_Debug_INFO
	if (Resource && !Name.empty())
	{
		Resource->SetName(Name);
	}
	else if (Resource)
	{
		Resource->SetName(GetUniqueName());
	}
#endif
}