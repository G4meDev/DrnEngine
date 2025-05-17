#pragma once

#include "ForwardTypes.h"

#define FUNC_DECLARE_MULTICAST_DELEGATE( MulticastDelegateName, ... ) \
	typedef MulticastDelegate<__VA_ARGS__> MulticastDelegateName;

#define DECLARE_MULTICAST_DELEGATE_OneParam( DelegateName, Param1Type )			\
	FUNC_DECLARE_MULTICAST_DELEGATE( DelegateName, Param1Type )

namespace Drn
{
	template <typename ...DelegateSignature>
	class MulticastDelegate
	{
	protected:
		using InvokationListType = std::function<void(DelegateSignature...)>;

		struct InvocationElement
		{
			InvocationElement(void* InClass, char* InName, InvokationListType InInvokation)
				: Class(InClass)
				, Name(InName)
				, Invokation(InInvokation)
			{
			}

			void* Class;
			InvokationListType Invokation;
			char* Name;
		};

	public:

		void Clear()
		{
			InvocationList.clear();
		}

		inline bool IsBound() const
		{
			return InvokationList.size() > 0;
		}

		template<class UserClass, class Func>
		inline void Add( UserClass* UClass, Func&& F, char* Name)
		{
			if constexpr ( sizeof...( DelegateSignature ) == 0)
			{
				InvocationElement e = InvocationElement( UClass, Name, std::bind( F, UClass) );
				InvokationList.push_back( e );
			}

			if constexpr ( sizeof...( DelegateSignature ) == 1)
			{
				// TODO: add move constructor
				InvocationElement e = InvocationElement( UClass, Name, std::bind( F, UClass, std::placeholders::_1 ) );
				InvokationList.push_back( e );
			}

		}

		template<class UserClass>
		inline void Remove( UserClass* UClass, const char* Name)
		{
			for ( auto it = InvokationList.begin(); it != InvokationList.end(); )
			{
				if (it->Class == UClass && strcmp( it->Name, Name) == 0 )
				{
					it = InvokationList.erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		inline void Braodcast( DelegateSignature... Pack ) const
		{
			for (auto& A : InvokationList)
			{
				A.Invokation(Pack...);
			}
		}

	private:

		std::vector<InvocationElement> InvokationList;
	};
}