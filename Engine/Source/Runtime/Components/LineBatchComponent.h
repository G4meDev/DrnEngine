#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	class LineBatchSceneProxy;

	struct BatchLine
	{
		BatchLine(const Vector& InStart, const Vector& InEnd, const Vector& InColor, float InThickness, float InLifetime)
			: Start(InStart)
			, End(InEnd)
			, Color(InColor)
			, Thickness(InThickness / 100.0f)
			, RemainingLifetime(InLifetime)
		{
		}

		Vector Start;
		Vector End;
		Vector Color;
		float Thickness;
		float RemainingLifetime;
	};

	class LineBatchComponent : public PrimitiveComponent
	{
	public:
		LineBatchComponent() : PrimitiveComponent() {}
		virtual ~LineBatchComponent() {};

		void TickComponent(float DeltaTime);

		std::vector<BatchLine> m_Lines;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		void DrawLine(const Vector& Start, const Vector& End, const Vector& Color, float Thickness, float Lifetime);
		void DrawLines(const std::vector<BatchLine>& InLines);

		void DrawCircle(const Vector& Base, const Vector& X, const Vector& Z, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawSphere(const Vector& Center, const Quat& Rotation, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);

		void DrawBox(const Box& InBox, const Transform& T, const Vector& Color, float Thickness, float Lifetime);

		void Flush();

		LineBatchSceneProxy* m_SceneProxy;
	};

	class LineBatchSceneProxy : public PrimitiveSceneProxy
	{
	public:

		LineBatchSceneProxy(LineBatchComponent* InLineBatchComponent);
		virtual ~LineBatchSceneProxy();

	protected:

		virtual void RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void InitResources(dx12lib::CommandList* CommandList) override;
		virtual void UpdateResources(dx12lib::CommandList* CommandList) override;

		virtual PrimitiveComponent* GetPrimitive() override { return m_LineComponent; }

	private:

		void RecalculateVertexData();
		void UploadVertexBuffer();

		LineBatchComponent* m_LineComponent;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

		std::vector<InputLayout_LineColorThickness> m_VertexData;
		std::vector<uint32> m_IndexData;

		AssetHandle<Material> m_LineBatchMaterial;

		bool m_HasValidData = false;
	};
}