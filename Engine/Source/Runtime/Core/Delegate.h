#pragma once

#include "ForwardTypes.h"

#define FUNC_DECLARE_DELEGATE( DelegateName, ReturnType, ... )							\
	typedef Delegate<ReturnType, __VA_ARGS__> DelegateName;

#define DECLARE_DELEGATE_RetVal( ReturnValueType, DelegateName )						\
	FUNC_DECLARE_DELEGATE( DelegateName, ReturnValueType )

#define DECLARE_DELEGATE( DelegateName )												\
	FUNC_DECLARE_DELEGATE( DelegateName, void )

#define DECLARE_DELEGATE_OneParam( DelegateName, Param1Type )							\
	FUNC_DECLARE_DELEGATE( DelegateName, void, Param1Type )

// ---------------------------------------------------------------------------------------------

#define FUNC_DECLARE_MULTICAST_DELEGATE( MulticastDelegateName, ReturnType, ... )		\
	typedef MulticastDelegate<ReturnType, __VA_ARGS__> MulticastDelegateName;

#define DECLARE_MULTICAST_DELEGATE_OneParam( DelegateName, Param1Type )					\
	FUNC_DECLARE_MULTICAST_DELEGATE( DelegateName, void, Param1Type )

namespace Drn
{
	template <typename ReturnType, typename ...DelegateSignature>
	class Delegate
	{
	protected:
		using InvokationListType = std::function<ReturnType(DelegateSignature...)>;

		struct InvocationElement
		{
			InvocationElement(void* InClass, InvokationListType InInvokation)
				: Class(InClass)
				, Invokation(InInvokation) { }

			InvocationElement()
				: Class(nullptr) { }

			void* Class;
			InvokationListType Invokation;
		};

	public:

		void Unbind()
		{
			InvocationElement.Class = nullptr;
		}

		inline bool IsBound() const
		{
			return Element.Class != nullptr;
		}

		template<class UserClass>
		inline bool IsBoundToClass( UserClass* UClass ) const
		{
			if (UClass && Element.Class == UClass)
			{
				return True;
			}

			return false;
		}

		template<class UserClass, class Func>
		inline void Bind( UserClass* UClass, Func&& F)
		{
			if constexpr ( sizeof...( DelegateSignature ) == 0)
			{
				Element = InvocationElement( UClass, std::bind( F, UClass ) );
			}

			if constexpr ( sizeof...( DelegateSignature ) == 1)
			{
				Element = InvocationElement( UClass, std::bind( F, UClass, std::placeholders::_1 ) );
			}

		}

		inline ReturnType Execute( DelegateSignature... Pack ) const
		{
			return Element.Invokation(Pack...);
		}

	private:

		InvocationElement Element;
	};

// -----------------------------------------------------------------------------------------------------

	template <typename ReturnType, typename ...DelegateSignature>
	class MulticastDelegate
	{
	protected:
		using InvokationListType = std::function<ReturnType(DelegateSignature...)>;

		struct InvocationElement
		{
			InvocationElement(void* InClass, InvokationListType InInvokation)
				: Class(InClass)
				, Invokation(InInvokation) { }

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
		inline void Add( UserClass* UClass, Func&& F)
		{
			if constexpr ( sizeof...( DelegateSignature ) == 0)
			{
				InvokationList.emplace_back( UClass, std::bind( F, UClass ) );
			}

			if constexpr ( sizeof...( DelegateSignature ) == 1)
			{
				InvokationList.emplace_back( UClass, std::bind( F, UClass, std::placeholders::_1 ) );
			}

		}

		template<class UserClass>
		inline void Remove( UserClass* UClass )
		{
			for ( auto it = InvokationList.begin(); it != InvokationList.end(); )
			{
				if (it->Class == UClass )
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