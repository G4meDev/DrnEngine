#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	class LineBatchSceneProxy;

	struct BatchLine
	{
		BatchLine(const Vector& InStart, const Vector& InEnd, const Vector& InColor, float InLifetime)
			: Start(InStart)
			, End(InEnd)
			, Color(InColor)
			, RemainingLifetime(InLifetime)
		{
		}

		Vector Start;
		Vector End;
		Vector Color;
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

		void DrawLine(const Vector& Start, const Vector& End, const Vector& Color, float Lifetime);
		void DrawLines(const std::vector<BatchLine>& InLines);

		void Flush();

		LineBatchSceneProxy* m_SceneProxy;
	};

	class LineBatchSceneProxy : public PrimitiveSceneProxy
	{
	public:

		LineBatchSceneProxy(LineBatchComponent* InLineBatchComponent);
		virtual ~LineBatchSceneProxy();

	protected:

		virtual void RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer ) const override;
		virtual void UpdateResources(dx12lib::CommandList* CommandList) override;

		virtual PrimitiveComponent* GetPrimitive() override { return m_LineComponent; }

	private:

		void UpdateBuffers( dx12lib::CommandList* CommandList );

		LineBatchComponent* m_LineComponent;

		std::shared_ptr<dx12lib::RootSignature> m_RootSignature = nullptr;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject = nullptr;

		std::shared_ptr<dx12lib::VertexBuffer> m_VertexBuffer;
		std::shared_ptr<dx12lib::IndexBuffer> m_IndexBuffer;

		std::vector<VertexData_Color> m_VertexData;
		std::vector<uint32> m_IndexData;

		bool m_HasValidData = false;
	};
}