#pragma once

#if WITH_EDITOR

namespace Drn
{
	class AssetPreview
	{
	public:
		AssetPreview(const std::string InPath);

		static AssetPreview* Create(const std::string InPath);

		inline std::string GetPath() { return m_Path; }

		virtual void SetCurrentFocus() = 0;

	protected:
		std::string m_Path;

	private:
	};
}

#endif