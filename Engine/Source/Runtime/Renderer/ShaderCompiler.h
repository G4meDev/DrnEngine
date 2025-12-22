#pragma once

#include "ForwardTypes.h"

namespace Drn
{
/*
	class DxcArguments
	{
	protected:
		std::string ShaderProfile;
		std::string EntryPoint;
		std::string Exports;
		std::string DumpDisasmFilename;
		std::string BatchBaseFilename;
		std::string DumpDebugInfoPath;
		bool bEnable16BitTypes = false;
		bool bDump = false;

		std::vector<std::string> ExtraArguments;

	public:
		DxcArguments(const std::string& InEntryPoint, const TCHAR* InShaderProfile, const std::string& InExports,
			const std::string& InDumpDebugInfoPath, const std::string& InBaseFilename, bool bInEnable16BitTypes,
			bool bKeepDebugInfo, uint32 D3DCompileFlags, uint32 AutoBindingSpace = ~0u)
			: ShaderProfile(InShaderProfile)
			, EntryPoint(InEntryPoint)
			, Exports(InExports)
			, DumpDebugInfoPath(InDumpDebugInfoPath)
			, bEnable16BitTypes(bInEnable16BitTypes)
		{
			BatchBaseFilename = Path:: GetBaseFilename(InBaseFilename);

			if (InDumpDebugInfoPath.Len() > 0)
			{
				bDump = true;
				DumpDisasmFilename = InDumpDebugInfoPath / TEXT("Output.d3dasm");
			}

			if (AutoBindingSpace != ~0u)
			{
				ExtraArguments.Add(L"/auto-binding-space");
				ExtraArguments.Add(std::string::Printf(TEXT("%d"), AutoBindingSpace));
			}

			if (Exports.Len() > 0)
			{
				// Ensure that only the requested functions exists in the output DXIL.
				// All other functions and their used resources must be eliminated.
				ExtraArguments.Add(L"/exports");
				ExtraArguments.Add(Exports);
			}

			if (D3DCompileFlags & D3D10_SHADER_PREFER_FLOW_CONTROL)
			{
				D3DCompileFlags &= ~D3D10_SHADER_PREFER_FLOW_CONTROL;
				ExtraArguments.Add(L"/Gfp");
			}

			if (D3DCompileFlags & D3D10_SHADER_SKIP_OPTIMIZATION)
			{
				D3DCompileFlags &= ~D3D10_SHADER_SKIP_OPTIMIZATION;
				ExtraArguments.Add(L"/Od");
			}

			if (D3DCompileFlags & D3D10_SHADER_SKIP_VALIDATION)
			{
				D3DCompileFlags &= ~D3D10_SHADER_SKIP_VALIDATION;
				ExtraArguments.Add(L"/Vd");
			}

			if (D3DCompileFlags & D3D10_SHADER_AVOID_FLOW_CONTROL)
			{
				D3DCompileFlags &= ~D3D10_SHADER_AVOID_FLOW_CONTROL;
				ExtraArguments.Add(L"/Gfa");
			}

			if (D3DCompileFlags & D3D10_SHADER_PACK_MATRIX_ROW_MAJOR)
			{
				D3DCompileFlags &= ~D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
				ExtraArguments.Add(L"/Zpr");
			}

			if (D3DCompileFlags & D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY)
			{
				D3DCompileFlags &= ~D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
				ExtraArguments.Add(L"/Gec");
			}

			switch (D3DCompileFlags & SHADER_OPTIMIZATION_LEVEL_MASK)
			{
			case D3D10_SHADER_OPTIMIZATION_LEVEL0:
				D3DCompileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL0;
				ExtraArguments.Add(L"/O0");
				break;

			case D3D10_SHADER_OPTIMIZATION_LEVEL1:
				D3DCompileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL1;
				ExtraArguments.Add(L"/O1");
				break;

			case D3D10_SHADER_OPTIMIZATION_LEVEL2:
				D3DCompileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL2;
				ExtraArguments.Add(L"/O2");
				break;

			case D3D10_SHADER_OPTIMIZATION_LEVEL3:
				D3DCompileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL3;
				ExtraArguments.Add(L"/O3");
				break;

			default:
				break;
			}

			if (D3DCompileFlags & D3D10_SHADER_DEBUG)
			{
				D3DCompileFlags &= ~D3D10_SHADER_DEBUG;
				bKeepDebugInfo = true;
			}

			if (bEnable16BitTypes)
			{
				ExtraArguments.Add(L"/enable-16bit-types");
			}

			checkf(D3DCompileFlags == 0, TEXT("Unhandled shader compiler flags 0x%x!"), D3DCompileFlags);


			ExtraArguments.Add(L"/Zss");
			ExtraArguments.Add(L"/Qembed_debug");
			ExtraArguments.Add(L"/Zi");
			ExtraArguments.Add(L"/Fd");
			ExtraArguments.Add(L".\\");

			// Reflection will be removed later, otherwise the disassembly won't contain variables
			//ExtraArguments.Add(L"/Qstrip_reflect");
		}

		inline std::string GetDumpDebugInfoPath() const
		{
			return DumpDebugInfoPath;
		}

		inline bool ShouldDump() const
		{
			return bDump;
		}

		std::string GetEntryPointName() const
		{
			return Exports.Len() > 0 ? std::string(L"") : EntryPoint;
		}

		const std::string& GetShaderProfile() const
		{
			return ShaderProfile;
		}

		const std::string& GetDumpDisassemblyFilename() const
		{
			return DumpDisasmFilename;
		}

		void GetCompilerArgsNoEntryNoProfileNoDisasm(TArray<const WCHAR*>& Out) const
		{
			for (const std::string& Entry : ExtraArguments)
			{
				Out.Add(*Entry);
			}
		}

		void GetCompilerArgs(TArray<const WCHAR*>& Out) const
		{
			GetCompilerArgsNoEntryNoProfileNoDisasm(Out);
			if (Exports.Len() == 0)
			{
				Out.Add(L"/E");
				Out.Add(*EntryPoint);
			}

			Out.Add(L"/T");
			Out.Add(*ShaderProfile);

			Out.Add(L" /Fc ");
			Out.Add(TEXT("zzz.d3dasm"));	// Dummy

			Out.Add(L" /Fo ");
			Out.Add(TEXT("zzz.dxil"));	// Dummy
		}

		std::string GetBatchCommandLineString(const std::string& ShaderPath) const
		{
			std::string DXCCommandline;
			for (const std::string& Entry : ExtraArguments)
			{
				DXCCommandline += L" ";
				DXCCommandline += Entry;
			}

			DXCCommandline += L" /T ";
			DXCCommandline += ShaderProfile;

			if (Exports.Len() == 0)
			{
				DXCCommandline += L" /E ";
				DXCCommandline += EntryPoint;
			}

			DXCCommandline += L" /Fc ";
			DXCCommandline += BatchBaseFilename + TEXT(".d3dasm");

			DXCCommandline += L" /Fo ";
			DXCCommandline += BatchBaseFilename + TEXT(".dxil");

			return DXCCommandline;
		}
	};

	class ShaderCompiler
	{
		static HRESULT D3DCompileToDxil(const char* SourceText, FDxcArguments& Arguments,
			TRefCountPtr<IDxcBlob>& OutDxilBlob, TRefCountPtr<IDxcBlob>& OutReflectionBlob, TRefCountPtr<IDxcBlobEncoding>& OutErrorBlob)

	};

*/
}