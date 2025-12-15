#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

constexpr uint32 MaxNumVertex = 2000000;

namespace Drn
{
	class LineBatchSceneProxy;

	struct BatchLine
	{
		BatchLine(const Vector& InStart, const Vector& InEnd, const Color& InColor, float InThickness, float InLifetime)
			: Start(InStart)
			, End(InEnd)
			, Color(InColor)
			, Thickness(InThickness / 100.0f)
			, RemainingLifetime(InLifetime)
		{
		}

		Vector Start;
		Vector End;

		// TODO: make color class of 8 bit
		Color Color;
		float Thickness;
		float RemainingLifetime;
	};

	class LineBatchComponent : public PrimitiveComponent
	{
	public:
		LineBatchComponent() : PrimitiveComponent()
			, m_Thickness(false)
			, m_SceneProxy(nullptr)
		{
			SetEditorPrimitive(true);
		}
		virtual ~LineBatchComponent() {};

		void TickComponent(float DeltaTime);

		std::vector<BatchLine> m_Lines;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		void DrawLine(const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Lifetime);
		void DrawLines(const std::vector<BatchLine>& InLines);

		void DrawCircle(const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawHalfCircle(const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawSphere(const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);

		void DrawBox(const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime);
		void DrawCapsule(const Vector& Center, float HalfHeight, float Radius, const Quat& Rotation, const Color& Color, float Thickness, float Lifetime);

		void Flush();

		virtual void SetThickness( bool InThickness );

		bool m_Thickness;

		LineBatchSceneProxy* m_SceneProxy;
	};

	class LineBatchSceneProxy : public PrimitiveSceneProxy
	{
	public:

		LineBatchSceneProxy(LineBatchComponent* InLineBatchComponent);
		virtual ~LineBatchSceneProxy();

		bool m_Thickness;
	protected:

		virtual void RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy);
		virtual void RenderDecalPass(class D3D12CommandList* CommandList, SceneRenderer* Renderer) override;


#if WITH_EDITOR
		virtual void RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
#endif

		virtual void InitResources(class D3D12CommandList* CommandList) override;
		virtual void UpdateResources(class D3D12CommandList* CommandList) override;

		virtual PrimitiveComponent* GetPrimitive() override { return m_LineComponent; }

	private:

		void RecalculateVertexData();
		void UploadVertexBuffer(class D3D12CommandList* CommandList);

		LineBatchComponent* m_LineComponent;

		TRefCountPtr<class RenderVertexBuffer> m_VertexBuffer;

		std::vector<InputLayout_LineColorThickness> m_VertexData;
		uint32 m_VertexCount;

		bool m_HasValidData = false;
	};
}