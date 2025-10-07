#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"

#define NUM_SCENE_DOWNSAMPLES 6

namespace Drn
{
	struct SceneDownSampleData
	{
	public:
		SceneDownSampleData() = default;

		Vector4 ParentSizeAndInvSize;

		uint32 ParentTexture;
	};

	class SceneDownSampleBuffer : public RenderBuffer
	{
	public:
		SceneDownSampleBuffer();
		virtual ~SceneDownSampleBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer );
		void ReleaseBuffers();

		Resource* m_DownSampleTargets[NUM_SCENE_DOWNSAMPLES] = { nullptr };

		DescriptorHandleRTV m_RTVHandles[NUM_SCENE_DOWNSAMPLES];
		DescriptorHandleSRV m_SrvHandles[NUM_SCENE_DOWNSAMPLES];

		Resource* m_Buffer[NUM_SCENE_DOWNSAMPLES] = {nullptr};
		SceneDownSampleData m_Data;

		D3D12_VIEWPORT m_Viewports[NUM_SCENE_DOWNSAMPLES];

	private:

#if D3D12_Debug_INFO
		inline std::string GetDownSamplePostfix(int32 Index) const { return "(1/" + std::to_string((int32)std::pow(2, Index + 2)) + ")"; }
#endif

	};
}