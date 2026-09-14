#pragma once

#include "ForwardTypes.h"
#include "Runtime/Animation/AnimationPose.h"

namespace Drn
{
	DECLARE_DELEGATE_RetVal( bool, AnimationTransitionDelegate );

	#define CREATE_AND_REGISTER_ANIMSTATE(owner,type,name,...) type* name = new type( __VA_ARGS__ ); owner->RegisterState(name);
	#define REGISTER_ANIMSTATE_TRANSITION(state,target,func) state->AddTransition(target, state, func);

	struct AnimationTransition
	{
		class AnimationState* TargetState;
		AnimationTransitionDelegate TransitionDelegate;
	};

	class AnimationState : public RefCountedObject
	{
	public:
		AnimationState(float InBlendInDuration = 0.05f, float InBlendOutDuration = 0.05f)
			: BlendInDuration(InBlendInDuration)
			, BlendOutDuration(InBlendOutDuration)
		{}

		virtual void Tick(float DeltaTime) = 0;

		virtual void OnEnterState() = 0;
		virtual void OnLeaveState() = 0;

		virtual const AnimationPose& GetPose() const = 0;

		template<class UserClass, class Func>
		void AddTransition(AnimationState* Target, UserClass* UClass, Func&& F)
		{
			drn_check(Target);

			Transitions.push_back({});
			Transitions.back().TargetState = Target;
			Transitions.back().TransitionDelegate.Bind(UClass, F);
		}

	protected:
		std::vector<AnimationTransition> Transitions;

		float BlendInDuration;
		float BlendOutDuration;

		friend class AnimationStateMachine;
	};

	class AnimationStateMachine : public RefCountedObject
	{
	public:
		virtual void Tick(float DeltaTime);

		virtual const AnimationPose& GetPose() const { return Pose; }

		inline void RegisterState(AnimationState* State) { States.push_back(State); };
		inline void SetDefaultState(AnimationState* State) { Transition(nullptr, State); }

	protected:
		void Transition(AnimationState* From, AnimationState* To);

		std::vector<TRefCountPtr<AnimationState>> States;
		AnimationState* CurrentState;
		AnimationState* PreviousState; // for blend transition

		AnimationPose Pose;

		float TimeSinceTransition;
	};
}