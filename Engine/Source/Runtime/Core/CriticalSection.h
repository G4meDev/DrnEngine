#pragma once

//#include <Windows.h>

namespace Drn
{
	class CriticalSection
	{
	public:
		CriticalSection(){}
	};

	class ScopeLock
	{
	public:
		ScopeLock(CriticalSection* InSynchObject){}
	};

/*
	class CriticalSection
	{
		CRITICAL_SECTION CS;

	public:

		__forceinline CriticalSection()
		{
			//CA_SUPPRESS(28125);
			::InitializeCriticalSection(&CS);
			::SetCriticalSectionSpinCount(&CS,4000);
		}

		__forceinline ~CriticalSection()
		{
			DeleteCriticalSection(&CS);
		}

		__forceinline void Lock()
		{
			EnterCriticalSection(&CS);
		}

		__forceinline bool TryLock()
		{
			return TryEnterCriticalSection(&CS);
		}

		__forceinline void Unlock()
		{
			LeaveCriticalSection(&CS);
		}

	private:
		CriticalSection(const CriticalSection&);
		CriticalSection& operator=(const CriticalSection&);
	};

	class ScopeLock
	{
	public:

		ScopeLock( CriticalSection* InSynchObject )
			: SynchObject(InSynchObject)
		{
			SynchObject->Lock();
		}

		~ScopeLock()
		{
			Unlock();
		}

		void Unlock()
		{
			if(SynchObject)
			{
				SynchObject->Unlock();
				SynchObject = nullptr;
			}
		}

	private:

		ScopeLock();
		ScopeLock(const ScopeLock& InScopeLock);
		ScopeLock& operator=( ScopeLock& InScopeLock )
		{
			return *this;
		}

	private:

		CriticalSection* SynchObject;
	};
*/
}