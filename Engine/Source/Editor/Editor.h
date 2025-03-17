#pragma once

#if WITH_EDITOR

LOG_DECLARE_CATEGORY(LogEditor);

namespace Drn
{

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Init();
		void Tick(float DeltaTime);

		static Editor* Get();

	protected:

	private:
		static std::unique_ptr<Editor> SingletonInstance;

	};

}

#endif