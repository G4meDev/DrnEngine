#pragma once

namespace Drn
{
	template<typename ContainerType>
	class LinkedListNode
	{
	public:
		LinkedListNode(ContainerType* InContent)
			: Content(InContent)
		{ };


		LinkedListNode()
			: LinkedListNode(nullptr)
		{ };

		void SetContent(ContainerType* InContent)
		{
			Content = InContent;
		}

		void SetNextNode(LinkedListNode<ContainerType>* InNextNode)
		{
			NextNode = InNextNode;
		}

		LinkedListNode* Next()
		{
			return NextNode;
		}

		ContainerType* operator*()
		{
			return Content;
		}

	private:
		LinkedListNode<ContainerType>* NextNode = nullptr;
		ContainerType* Content;
	};


	template<typename ContainerType>
	class LinkedList
	{
	public:
		LinkedList()
		{ };

		void AddFirst(ContainerType* InElement)
		{
			LinkedListNode<ContainerType>* Node = new LinkedListNode<ContainerType>(InElement);

			Node->SetNextNode(Root);
			Root = Node;
		}

		LinkedListNode<ContainerType>* Root = nullptr;
	};


	template<typename ContainerType>
	class LinkedListIterator
	{
	public:
		LinkedListIterator(LinkedList<ContainerType>& InList)
			: CurrentNode(InList.Root), List(&InList)
		{ }

		void Next()
		{
			if (!bRemovedElement)
			{
				PrevNode = CurrentNode;
				CurrentNode = CurrentNode->Next();
			}

			bRemovedElement = false;
		}

		void Remove()
		{
			// removing root
			if (!PrevNode)
			{
				List->Root = CurrentNode->Next();
			}

			LinkedListNode<ContainerType>* TempCurrent = CurrentNode;
			CurrentNode = CurrentNode->Next();
			delete TempCurrent;

			bRemovedElement = true;
		}

		LinkedListIterator& operator++()
		{
			Next();
			return *this;
		}

		operator bool() const
		{
			return CurrentNode != nullptr;
		}

		ContainerType* operator->() const
		{
			return **(this->CurrentNode);
		}

		ContainerType* operator*() const
		{
			return **(this->CurrentNode);
		}

	private:
		LinkedList<ContainerType>* List;

		LinkedListNode<ContainerType>* CurrentNode;
		LinkedListNode<ContainerType>* PrevNode = nullptr;

		bool bRemovedElement = false;
	};
}