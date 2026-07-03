#include "DrnPCH.h"
#include "ParticleSystemComponent.h"

#if WITH_EDITOR
#include "Editor/EditorConfig.h"
#endif

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
			std::string TemplatePath = "";
			Ar >> TemplatePath;
			SetTemplate(AssetHandle<ParticleSystem>(TemplatePath));
		}

		else
		{
			Ar << Template.GetPath();
		}

	}

	void ParticleSystemComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_ParticleIcon.drn" );
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
		Template = InTemplate;
		Template.Load();

		ResetEmitters();
	}

	bool ParticleSystemComponent::IsUsingTemplate( AssetHandle<ParticleSystem> InTemplate )
	{
		return Template.IsValid() && (Template.GetPath() == InTemplate.GetPath());
	}

	void ParticleSystemComponent::ResetEmitters()
	{
		Emitters.clear();

		if (Template.IsValid())
		{
			for (int32 i = 0; i < Template->Emitters.size(); i++)
			{
				ParticleEmitter* Emitter = Template->Emitters[i];
				if (Emitter->IsEnabled())
				{
					TRefCountPtr<ParticleMeshEmitterInstance> MeshEmitter = new ParticleMeshEmitterInstance();
					Emitters.push_back((ParticleEmitterInstance*)MeshEmitter);
		
					MeshEmitter->InitParameters(Emitter, this);
					MeshEmitter->Init();
				}
			}
		}

		//TRefCountPtr<ParticleMeshEmitterInstance> MeshEmitter = new ParticleMeshEmitterInstance();
		//Emitters.push_back((ParticleEmitterInstance*)MeshEmitter);
		//
		//MeshEmitter->InitParameters(nullptr, this);
		//MeshEmitter->Init();
	}

#if WITH_EDITOR
	void ParticleSystemComponent::DrawDetailPanel( float DeltaTime )
	{
		if ( ImGui::Button( "Clear" ) )
		{
			SetTemplate(AssetHandle<ParticleSystem>(""));
		}

		std::string AssetPath	= Template.GetPath();
		std::string AssetName	= Path::ConvertShortPath(AssetPath);
		AssetName				= Path::RemoveFileExtension(AssetName);
		AssetName				= AssetName == "" ? "None" : AssetName;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, EditorConfig::AssetInputColor);
		ImGui::Text( "%s", AssetName.c_str() );
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
			{
				auto AssetPath = static_cast<const char*>(payload->Data);
				AssetHandle<Asset> NewAsset(AssetPath);
				EAssetType Type = NewAsset.LoadGeneric();

				if (NewAsset.IsValid() && Type == EAssetType::ParticleSystem)
				{
					AssetHandle<ParticleSystem> TypedAsset(AssetPath);
					TypedAsset.Load();

					SetTemplate(TypedAsset);
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();
		ImGui::TextWrapped(Template.GetPath().c_str());
	}

	void ParticleSystemComponent::DrawEditorDefault()
	{
		
	}

	void ParticleSystemComponent::DrawEditorSelected()
	{
		
	}
#endif
}  // namespace Drn