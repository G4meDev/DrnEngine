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
		using InvocationListType = std::function<void(DelegateSignature...)>;

	public:

		void Clear()
		{
			InvocationList.clear();
		}

		inline bool IsBound() const
		{
			return true;
		}

		template<class UserClass, class Func>
		inline void Add( UserClass* UClass, Func&& F)
		{
			if constexpr ( sizeof...( DelegateSignature ) == 0)
			{
				InvocationList.push_back( std::bind( F, UClass ) );
			}

			if constexpr ( sizeof...( DelegateSignature ) == 1)
			{
				InvocationList.push_back( std::bind( F, UClass, std::placeholders::_1 ) );
			}

		}

		template<class UserClass, class Func>
		inline void Remove( UserClass* UClass, Func&& F)
		{
			//InvocationList.erase( UClass );

			//auto a = std::bind( F, UClass, std::placeholders::_1 ); 
			//InvocationList.erase(  );
			
			//if constexpr ( sizeof...( DelegateSignature ) == 0)
			//{
			//	InvocationList.erase( std::bind( F, UClass ) );
			//}
			//
			//if constexpr ( sizeof...( DelegateSignature ) == 1)
			//{
			//	InvocationList.erase( std::bind( F, UClass, std::placeholders::_1 ) );
			//}

		}

		inline void Braodcast( DelegateSignature... Pack ) const
		{
			for (auto& A : InvocationList)
			{
				A(Pack...);
			}
		}

	private:

		std::vector<InvocationListType> InvocationList;
	};
}