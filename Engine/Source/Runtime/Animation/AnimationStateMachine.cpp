#include "DrnPCH.h"
#include "AnimationStateMachine.h"

namespace Drn
{
	void AnimationStateMachine::Tick( float DeltaTime )
	{
		drn_check(CurrentState);

		CurrentState->Tick(DeltaTime);

		const float BlendDuration = PreviousState ? PreviousState->BlendOutDuration + CurrentState->BlendInDuration : -1.0f;
		const bool bNeedsBlending = TimeSinceTransition < BlendDuration;

		if (bNeedsBlending)
		{
			PreviousState->Tick(DeltaTime);

			float BlendAlpha = TimeSinceTransition / BlendDuration;
			Pose = AnimationPose::Blend(PreviousState->GetPose(), CurrentState->GetPose(), BlendAlpha);
		}

		else
		{
			Pose = CurrentState->GetPose();
		}

		TimeSinceTransition += DeltaTime;

		for (const AnimationTransition& T : CurrentState->Transitions)
		{
			drn_check(T.TargetState);
			drn_check(T.TransitionDelegate.IsBound());

			if (T.TransitionDelegate.Execute())
			{
				Transition(CurrentState, T.TargetState);
			}
		}
	}

	void AnimationStateMachine::Transition(AnimationState* From, AnimationState* To)
	{
		drn_check(To);
		drn_check(From != To);

		if (From)
		{
			From->OnLeaveState();
		}
		To->OnEnterState();

		PreviousState = From;
		CurrentState = To;

		TimeSinceTransition = 0.0f;
	}

        }