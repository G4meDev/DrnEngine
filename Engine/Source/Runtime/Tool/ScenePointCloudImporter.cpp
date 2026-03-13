#include "DrnPCH.h"
#include "ScenePointCloudImporter.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	ScenePointCloudImporter::ScenePointCloudImporter()
	{
		m_RootComponent = std::make_unique<class SceneComponent>();
		m_RootComponent->SetComponentLabel( "RootComponent" );
		SetRootComponent(m_RootComponent.get());

		//FilesPath = {"Game\\Content\\Level_2\\LevelBlockout\\Generated.json"};
	}

	ScenePointCloudImporter::~ScenePointCloudImporter()
	{
		
	}

	void ScenePointCloudImporter::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		//m_RootComponent->Serialize(Ar);

		if (Ar.IsLoading())
		{
			int32 NumPath;
			Ar >> NumPath;
			
			FilesPath.resize(NumPath);
			for (int32 Index = 0; Index < NumPath; Index++)
			{
				Ar >> FilesPath[Index];
			}
		}

		else
		{
			int32 NumPath = FilesPath.size();
			Ar << NumPath;
			
			for (int32 Index = 0; Index < NumPath; Index++)
			{
				Ar << FilesPath[Index];
			}
		}
	}

#if WITH_EDITOR
	void DestroyGenerated(World* Context)
	{
		auto& Actors = Context->GetActorList();
		for (Actor* A : Actors)
		{
			if (A->ActorHasTag("Generated"))
			{
				A->Destroy();
			}
		}
	}

	bool ScenePointCloudImporter::DrawDetailPanel()
	{
		bool Dirty = Actor::DrawDetailPanel();

		if (ImGui::Button("Import"))
		{
			DestroyGenerated(GetWorld());

			for (int32 Index = 0; Index < FilesPath.size(); Index++)
			{
				Import(FilesPath[Index]);
			}
		}

		if (ImGui::Button("Add"))
		{
			FilesPath.push_back("");
		}

		if (ImGui::Button("Remove"))
		{
			if (!FilesPath.empty())
			{
				FilesPath.pop_back();
			}
		}

		if (ImGui::Button("Clear"))
		{
			FilesPath.clear();
		}

		const int32 NumFiles = FilesPath.size();
		for (int32 Index = 0; Index < NumFiles; Index++)
		{
			char Path[128];
			strcpy(Path, FilesPath[Index].c_str());

			ImGui::PushID(Index);
			if (ImGui::InputText("##", Path, 128))
			{
				FilesPath[Index] = Path;
			}
			ImGui::PopID();
		}

		return Dirty;
	}

	Vector ReadVector(rapidjson::Value& Input)
	{
		auto Arr = Input.GetArray();
		return Vector(Arr[0].GetFloat(), Arr[1].GetFloat(), Arr[2].GetFloat());
	}

	Vector4 ReadVector4(rapidjson::Value& Input)
	{
		auto Arr = Input.GetArray();
		return Vector4(Arr[0].GetFloat(), Arr[1].GetFloat(), Arr[2].GetFloat(), Arr[3].GetFloat());
	}

	Quat ReadQuat(rapidjson::Value& Input)
	{
		auto Arr = Input.GetArray();
		return Quat(Arr[0].GetFloat(), Arr[1].GetFloat(), Arr[2].GetFloat(), Arr[3].GetFloat());
	}

	Transform ReadTransform(rapidjson::Value& Input)
	{
		rapidjson::Value& Location = Input["Location"];
		rapidjson::Value& Rotation = Input["Rotation"];
		rapidjson::Value& Scale = Input["Scale"];

		return Transform(ReadVector(Location), ReadQuat(Rotation), ReadVector(Scale));
	}

	void ImportInstancedStaticMeshes(rapidjson::Document& Document, World* Context)
	{
		rapidjson::Value& InstancedStaticMeshes = Document["InstancedStaticMeshes"];
		
		auto Arr = InstancedStaticMeshes.GetArray();
		for (auto& A : Arr)
		{
			rapidjson::Value& DisplayName = A["DisplayName"];

			rapidjson::Value& MeshPath = A["Mesh"];
			AssetHandle<StaticMesh> Mesh(MeshPath.GetString());
			Mesh.Load();

			rapidjson::Value& T = A["ActorTransform"];
			Transform ActorTransform = ReadTransform(T);

			rapidjson::Value& InstancesTransforms = A["InstanceTransforms"];
			auto InstancesTransformsArr = InstancesTransforms.GetArray();
			const int32 InstanceCount = InstancesTransforms.Size();
			std::vector<Transform> IT;
			IT.reserve(InstanceCount);
			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; InstanceIndex++)
			{
				IT.push_back(ReadTransform(InstancesTransformsArr[InstanceIndex]));
			}

			InstancedStaticMeshActor* SpawnedActor = Context->SpawnActor<InstancedStaticMeshActor>();
			SpawnedActor->SetActorTransform(ActorTransform);
			SpawnedActor->SetActorLabel(DisplayName.GetString());

			SpawnedActor->GetInstancedStaticMeshComponent()->AddInstances(IT, false);
			SpawnedActor->GetInstancedStaticMeshComponent()->SetMesh(Mesh);

			SpawnedActor->AddActorTag("Generated");
		}
	}

	void ScenePointCloudImporter::Import( const std::string& FilePath )
	{
		std::string AbsolutePath = Path::ConvertProjectPath(FilePath);
		drn_check(FileSystem::FileExists(AbsolutePath));

		std::string FileString = FileSystem::ReadFileAsString(AbsolutePath);
		rapidjson::Document Document;
		Document.Parse(FileString.c_str());

		ImportInstancedStaticMeshes(Document, GetWorld());
	}

#endif
}  // namespace Drn