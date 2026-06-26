#include "DrnPCH.h"
#include "ParticleSystemComponent.h"

namespace Drn
{
	ParticleSystemComponent::ParticleSystemComponent()
	{
		bTickInEditor = true;
		
	}

	ParticleSystemComponent::~ParticleSystemComponent()
	{
		
	}

	void ParticleSystemComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);

		for (int32 EmitterIndex = 0; EmitterIndex < Emitters.size(); EmitterIndex++)
		{
			ParticleEmitterInstance* Instance = Emitters[EmitterIndex];

			if (EmitterIndex + 1 < Emitters.size())
			{
				ParticleEmitterInstance* NextInstance = Emitters[EmitterIndex+1];
				ApplicationMisc::Prefetch(NextInstance);
			}

			//if (Instance && Instance->Emitter)
			{
				Instance->Tick(DeltaTime);
			}
		}
	}

	void ParticleSystemComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if ( Ar.IsLoading() )
		{
			Ar >> TestParam;

			SetTemplate(AssetHandle<ParticleSystem>(""));
		}

		else
		{
			Ar << TestParam;
		}

	}

	void ParticleSystemComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\T_DefaultComponentIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void ParticleSystemComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();
	}

	void ParticleSystemComponent::SetTemplate( AssetHandle<ParticleSystem> InTemplate )
	{
		//Template = InTemplate;
		//Template.Load();

		ResetEmitters();
	}

	void ParticleSystemComponent::ResetEmitters()
	{
		Emitters.clear();

		TRefCountPtr<ParticleMeshEmitterInstance> MeshEmitter = new ParticleMeshEmitterInstance();
		Emitters.push_back((ParticleEmitterInstance*)MeshEmitter);

		MeshEmitter->InitParameters(nullptr, this);
		MeshEmitter->Init();
	}

#if WITH_EDITOR
	void ParticleSystemComponent::DrawDetailPanel( float DeltaTime )
	{
		ImGui::DragFloat("Test", &TestParam, 0.01f, 0.001f, 10.0f, "%.2f");

	}

	void ParticleSystemComponent::DrawEditorDefault()
	{
		
	}

	void ParticleSystemComponent::DrawEditorSelected()
	{
		
	}
#endif
}  // namespace Drn