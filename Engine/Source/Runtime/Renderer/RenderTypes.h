#pragma once

#include "ForwardTypes.h"

#define MAX_VERTEX_ELEMENT_COUT 16

namespace Drn
{
	enum EShaderType : uint8
	{
		Vertex			= 0,
		Hull			= 1,
		Domain			= 2,
		Pixel			= 3,
		Geometry		= 4,
		Compute			= 5,
		RayGen			= 6,
		RayMiss			= 7,
		RayHitGroup		= 8,
		RayCallable		= 9,
		Max				= 10
	};

	enum class EBlendOperation : uint8
	{
		Add,
		Subtract,
		Min,
		Max,
		ReverseSubtract,
	};

	enum class EBlendFactor : uint8
	{
		Zero,
		One,
		SourceColor,
		InverseSourceColor,
		SourceAlpha,
		InverseSourceAlpha,
		DestAlpha,
		InverseDestAlpha,
		DestColor,
		InverseDestColor,
		ConstantBlendFactor,
		InverseConstantBlendFactor,
		Source1Color,
		InverseSource1Color,
		Source1Alpha,
		InverseSource1Alpha,
	};

	enum class ERasterizerFillMode : uint8
	{
		Wireframe,
		Solid,
	};

	enum class ERasterizerCullMode : uint8
	{
		None,
		Front,
		Back,
	};

	enum class EColorWriteMask : uint8
	{
		RED   = 0x01,
		GREEN = 0x02,
		BLUE  = 0x04,
		ALPHA = 0x08,

		NONE  = 0,
		RGB   = RED | GREEN | BLUE,
		RGBA  = RED | GREEN | BLUE | ALPHA,
		RG    = RED | GREEN,
		BA    = BLUE | ALPHA,
	};

	enum class ECompareFunction : uint8
	{
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Equal,
		NotEqual,
		Never,
		Always,
	};

	enum class EStencilMask : uint8
	{
		Bit_Default,
		Bit_255,
		Bit_1,
		Bit_2,
		Bit_4,
		Bit_8,
		Bit_16,
		Bit_32,
		Bit_64,
		Bit_128,
	};

	enum class EStencilOp : uint8
	{
		Keep,
		Zero,
		Replace,
		SaturatedIncrement,
		SaturatedDecrement,
		Invert,
		Increment,
		Decrement,
	};

	enum class EPrimitiveType : uint8
	{
		TriangleList,
		TriangleStrip,
		LineList,
		PointList
	};

	enum class EPrimitiveTopologyType : uint8
	{
		Triangle,
		Patch,
		Line,
		Point,
	};

	enum class EBufferUsageFlags : uint32
	{
		None					= 0,
	
		// The buffer will be written to once.
		Static					= 1 << 0, 
 
		// The buffer will be written to occasionally, GPU read only, CPU write only.  The data lifetime is until the next update, or the buffer is destroyed.
		Dynamic					= 1 << 1, 

		// The buffer's data will have a lifetime of one frame.  It MUST be written to each frame, or a new one created each frame.
		Volatile				= 1 << 2, 

		UnorderedAccess			= 1 << 3, 

		// Create a byte address buffer, which is basically a structured buffer with a uint32 type.
		ByteAddressBuffer		= 1 << 4,

		// Buffer that the GPU will use as a source for a copy.
		SourceCopy				= 1 << 5,

		// Create a buffer that can be bound as a stream output target.
		StreamOutput			= 1 << 6,

		// Create a buffer which contains the arguments used by DispatchIndirect or DrawIndirect.
		DrawIndirect			= 1 << 7,

		/** 
		 * Create a buffer that can be bound as a shader resource. 
		 * This is only needed for buffer types which wouldn't ordinarily be used as a shader resource, like a vertex buffer.
		 */
		ShaderResource			= 1 << 8,

		/** Buffer should be allocated from transient memory. */
		Transient				= 1 << 9,

		AccelerationStructure	= 1 << 10,

		VertexBuffer			= 1 << 11,
		IndexBuffer				= 1 << 12,
		StructuredBuffer		= 1 << 13,

		// Helper bit-masks
		AnyDynamic = (Dynamic | Volatile),
	};

	enum class EUniformBufferUsage : uint8
	{
		SingleDraw = 0,
		SingleFrame,
		MultiFrame,
	};

	enum ETextureCreateFlags
	{
		None							= 0,
		RenderTargetable				= 1<<0,
		ResolveTargetable				= 1<<1,
		DepthStencilTargetable			= 1<<2,
		ShaderResource					= 1<<3,
		SRGB							= 1<<4,
		CPUWritable						= 1<<5,
		DisableSRVCreation				= 1<<6,
		UAV								= 1<<7,
		Presentable						= 1<<8,
		CPUReadback						= 1<<9,
		TargetArraySlicesIndependently	= 1<<10,
		NoFastClear						= 1<<11,
		DepthStencilResolveTarget		= 1<<12,
		Transient						= 1<<13,
	};

	enum class EDepthStencilViewType : uint8
	{
		DepthStencilRead		= 0,
		DepthWrite				= 1,
		StencilWrite			= 2,
		DepthStencilWrite		= 3,
		Max						= 4
	};

	struct CopyTextureInfo
	{
		IntVector Size = IntVector::Zero;

		IntVector SourcePosition = IntVector::Zero;
		IntVector DestPosition = IntVector::Zero;

		uint32 SourceSliceIndex = 0;
		uint32 DestSliceIndex = 0;
		uint32 NumSlices = 1;

		uint32 SourceMipIndex = 0;
		uint32 DestMipIndex = 0;
		uint32 NumMips = 1;
	};

	enum class EClearBinding
	{
		ENoneBound,
		EColorBound,
		EDepthStencilBound,
	};

	struct ClearValueBinding
	{
		struct DSVAlue
		{
			float Depth;
			uint32 Stencil;
		};

		ClearValueBinding()
			: ColorBinding(EClearBinding::EColorBound)
		{
			Value.Color[0] = 0.0f;
			Value.Color[1] = 0.0f;
			Value.Color[2] = 0.0f;
			Value.Color[3] = 0.0f;
		}

		ClearValueBinding(EClearBinding NoBinding)
			: ColorBinding(NoBinding)
		{
			drn_check(ColorBinding == EClearBinding::ENoneBound);
		}

		explicit ClearValueBinding(const Vector4& InClearColor) // TODO: Linear color
			: ColorBinding(EClearBinding::EColorBound)
		{
			Value.Color[0] = InClearColor.GetX();
			Value.Color[1] = InClearColor.GetY();
			Value.Color[2] = InClearColor.GetZ();
			Value.Color[3] = InClearColor.GetW();
		}

		explicit ClearValueBinding(float DepthClearValue, uint32 StencilClearValue = 0)
			: ColorBinding(EClearBinding::EDepthStencilBound)
		{
			Value.DSValue.Depth = DepthClearValue;
			Value.DSValue.Stencil = StencilClearValue;
		}

		Vector4 GetClearColor() const // TODO: Linear color
		{
			drn_check(ColorBinding == EClearBinding::EColorBound);
			return Vector4(Value.Color[0], Value.Color[1], Value.Color[2], Value.Color[3]);
		}

		void GetDepthStencil(float& OutDepth, uint32& OutStencil) const
		{
			drn_check(ColorBinding == EClearBinding::EDepthStencilBound);
			OutDepth = Value.DSValue.Depth;
			OutStencil = Value.DSValue.Stencil;
		}

		bool operator==(const ClearValueBinding& Other) const
		{
			if (ColorBinding == Other.ColorBinding)
			{
				if (ColorBinding == EClearBinding::EColorBound)
				{
					return
						Value.Color[0] == Other.Value.Color[0] &&
						Value.Color[1] == Other.Value.Color[1] &&
						Value.Color[2] == Other.Value.Color[2] &&
						Value.Color[3] == Other.Value.Color[3];

				}
				if (ColorBinding == EClearBinding::EDepthStencilBound)
				{
					return
						Value.DSValue.Depth == Other.Value.DSValue.Depth &&
						Value.DSValue.Stencil == Other.Value.DSValue.Stencil;
				}
				return true;
			}
			return false;
		}

		EClearBinding ColorBinding;

		union ClearValueType
		{
			float Color[4];
			DSVAlue DSValue;
		} Value;

		static const ClearValueBinding None;
		static const ClearValueBinding Black;
		static const ClearValueBinding BlackZeroAlpha;
		static const ClearValueBinding BlackMaxAlpha;
		static const ClearValueBinding White;
		static const ClearValueBinding Transparent;
		static const ClearValueBinding DepthOne;
		static const ClearValueBinding DepthZero;
		static const ClearValueBinding DepthNear;
		static const ClearValueBinding DepthFar;	
		static const ClearValueBinding Green;
		static const ClearValueBinding DefaultNormal8Bit;
	};

	struct RenderResourceCreateInfo
	{
		RenderResourceCreateInfo()
			: BulkData(nullptr)
			, ResourceArray(nullptr)
			, ClearValueBinding(Vector4(0))
			, DebugName("<unnamed_resource>")
		{}

		RenderResourceCreateInfo(void* InBlukData, void* InResourceArray, const ClearValueBinding& InClearBinding, const std::string& InDebugName)
			: BulkData(InBlukData)
			, ResourceArray(InResourceArray)
			, ClearValueBinding(InClearBinding)
			, DebugName(InDebugName)
		{}

		RenderResourceCreateInfo(void* InBulkData)
			: RenderResourceCreateInfo()
		{
			BulkData = InBulkData;
		}

		//RenderResourceCreateInfo(void* InResourceArray)
		//	: RenderResourceCreateInfo()
		//{
		//	ResourceArray = InResourceArray;
		//}

		RenderResourceCreateInfo(const ClearValueBinding& InClearValueBinding)
			: RenderResourceCreateInfo()
		{
			ClearValueBinding = InClearValueBinding;
		}

		RenderResourceCreateInfo(const std::string& InDebugName)
			: RenderResourceCreateInfo()
		{
			DebugName = InDebugName;
		}

		//FResourceBulkDataInterface* BulkData;
		//FResourceArrayInterface* ResourceArray;

		void* BulkData;
		void* ResourceArray;

		ClearValueBinding ClearValueBinding;

		std::string DebugName;
	};
}