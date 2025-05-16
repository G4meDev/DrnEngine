#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"
#include <ImGuizmo.h>

namespace Drn
{
	enum class ETranslationSnap : uint8
	{
		NoSnap,
		Snap_01,
		Snap_05,
		Snap_1,
		Snap_10,
		MAX
	};

	enum class ERotationSnap : uint8
	{
		NoSnap,
		Snap_1,
		Snap_5,
		Snap_30,
		Snap_45,
		MAX
	};

	enum class EScaleSnap : uint8
	{
		NoSnap,
		Snap_025,
		Snap_1,
		Snap_5,
		Snap_10,
		MAX
	};

	enum class EGizmoSpace : uint16
	{
		Translation		= (1u << 0) | (1u << 1) | (1u << 2),
		Rotation		= (1u << 3) | (1u << 4) | (1u << 5),
		Scale			= (1u << 7) | (1u << 8) | (1u << 9)
	};

	enum class EGizmoMode : uint8
	{
		Local,
		World
	};

	struct GizmoState
	{
	public:

		GizmoState()
			: m_Space(EGizmoSpace::Translation)
			, m_Mode(EGizmoMode::Local)
			, m_TransitionSnap(ETranslationSnap::Snap_1)
			, m_ScaleSnap(EScaleSnap::Snap_025)
			, m_RotationSnap(ERotationSnap::Snap_30)
		{
		}

		inline IMGUIZMO_NAMESPACE::OPERATION GetGuizmoSpace() const { return static_cast<IMGUIZMO_NAMESPACE::OPERATION>(m_Space); }
		inline IMGUIZMO_NAMESPACE::MODE GetGuizmoMode() const { return static_cast<IMGUIZMO_NAMESPACE::MODE>(m_Mode); }

		void GetSnapValue( float* SnapValue ) const;

		void Draw();
		void HandleInput();

		std::string GetTransitionSnapName(ETranslationSnap Snap) const;
		std::string GetRotationSnapName(ERotationSnap Snap) const;
		std::string GetScaleSnapName(EScaleSnap Snap) const;

		float GetTransitionSnapValue(ETranslationSnap Snap) const;
		float GetRotationSnapValue(ERotationSnap Snap) const;
		float GetScaleSnapValue(EScaleSnap Snap) const;

		EGizmoSpace			m_Space;
		EGizmoMode			m_Mode;

		ETranslationSnap	m_TransitionSnap;
		ERotationSnap		m_RotationSnap;
		EScaleSnap			m_ScaleSnap;
	};
}
#endif