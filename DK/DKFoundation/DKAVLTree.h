//
//  File: DKAVLTree.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include <new>
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKFunction.h"

////////////////////////////////////////////////////////////////////////////////
// DKAVLTree
// AVL(Georgy Adelson-Velsky and Landis') tree template implementation.
//
// do balancing when weights diff > 2
//
// thread-safety: add, remove, find
// If you need modify value directly, object should be locked previously.
//
// VALUE: value-type
// KEY: key-type for searching
// CMPV: value to value comparison function or function object.
// CMPK: value to key comparison function or function object. (searching only)
// COPY: copy value function or function object.
//   (used when Update() called, You can ignore this type if you don't call Update)
//
// Note:
//  value's pointer will not be changed after balancing process.
//  You can save pointer if you wish.
////////////////////////////////////////////////////////////////////////////////


namespace DKFoundation
{
	template <typename VALUE, typename KEY> struct DKTreeComparison
	{
		int operator () (const VALUE& lhs, const KEY& rhs) const
		{
			if (lhs > rhs)				return 1;
			else if (lhs < rhs)			return -1;
			return 0;
		}
	};
	template <typename VALUE> struct DKTreeValueCopy
	{
		void operator () (VALUE& dst, const VALUE& src) const
		{
			dst = src;
		}
	};

	template <
		typename VALUE,									// value-type
		typename KEY,									// key-type
		typename CMPV = DKTreeComparison<VALUE, VALUE>,	// value comparison
		typename CMPK = DKTreeComparison<VALUE, KEY>,	// key-value comparison
		typename COPY = DKTreeValueCopy<VALUE>,			// value copy
		typename ALLOC = DKMemoryDefaultAllocator		// memory allocator
	>
	class DKAVLTree
	{
	public:
		typedef VALUE				Value;
		typedef KEY					Key;
		typedef CMPV				ValueCompare;
		typedef CMPK				KeyCompare;
		typedef COPY				ValueCopy;
		typedef ALLOC				Allocator;

		typedef DKTypeTraits<Value>		ValueTraits;
		typedef DKTypeTraits<Key>	SearchTratis;

		DKAVLTree(void)
			: rootNode(NULL), count(0)
		{
		}
		DKAVLTree(DKAVLTree&& tree)
			: rootNode(NULL), count(0)
		{
			rootNode = tree.rootNode;
			count = tree.count;
			tree.rootNode = NULL;
			tree.count = 0;
		}
		// Copy constructor. accepts same type of class.
		// templates not works on MSVC (bug?)
		DKAVLTree(const DKAVLTree& s)
			: rootNode(NULL), count(0)
		{
			if (s.rootNode)
				rootNode = s.rootNode->Duplicate();
			count = s.count;
		}
		~DKAVLTree(void)
		{
			Clear();
		}
		// Update: insertion if not exist or overwrite if exists.
		const Value* Update(const Value& v)
		{
			bool created = false;
			Node* node = SetNode(v, &created);
			if (!created)
				valueCopyFunc(node->value, v);
			return &node->value;
		}
		// Insert: insert if not exist or fail if exists.
		//  returns NULL if function failed. (already exists)
		const Value* Insert(const Value& v)
		{
			bool created = false;
			Node* node = SetNode(v, &created);
			if (created && node)
				return &node->value;
			return NULL;
		}
		void Remove(const Key& k)
		{
			Node* node = GetNode(k);
			if (node == NULL)
				return;

			Node* retrace = NULL;	// entry node to begin rotation.

			if (node->left && node->right)
			{
				Node* replace = NULL;

				// find biggest from left, smallest from right, and swap, remove node.
				// after remove, rotate again

				if (node->leftHeight > node->rightHeight)
				{
					// finding biggest from left and swap.
					for (replace = node->left; replace->right; replace = replace->right);
					if (replace == node->left) // replacement node (child-node)
					{
						retrace = replace;
					}
					else
					{
						retrace = replace->parent;
						// set 'retrace's right-node to 'replace's left-node
						retrace->right = replace->left;
						if (retrace->right)
							retrace->right->parent = retrace;
						replace->left = node->left;
						replace->left->parent = replace;
					}
					replace->right = node->right;
					replace->right->parent = replace;
				}
				else
				{
					// finding smallest from right and swap.
					for (replace = node->right; replace->left; replace = replace->left);
					if (replace == node->right) // replacement node (child-node)
					{
						retrace = replace;
					}
					else
					{
						retrace = replace->parent;
						// set 'replace's right-node to 'retrace's left-node
						retrace->left = replace->right;
						if (retrace->left)
							retrace->left->parent = retrace;
						replace->right = node->right;
						replace->right->parent = replace;
					}
					replace->left = node->left;
					replace->left->parent = replace;
				}
				// set 'node's parent with 'replace' as child-node
				if (node->parent)
				{
					if (node->parent->left == node)
						node->parent->left = replace;
					else
						node->parent->right = replace;
				}
				replace->parent = node->parent;
			}
			else
			{
				retrace = node->parent;

				if (retrace)
				{
					if (retrace->left == node)
					{
						if (node->left)
						{
							node->left->parent = retrace;
							retrace->left = node->left;
						}
						else if (node->right)
						{
							node->right->parent = retrace;
							retrace->left = node->right;
						}
						else
							retrace->left = NULL;
					}
					else
					{
						if (node->left)
						{
							node->left->parent = retrace;
							retrace->right = node->left;
						}
						else if (node->right)
						{
							node->right->parent = retrace;
							retrace->right = node->right;
						}
						else
							retrace->right = NULL;
					}
				}
				else
				{
					if (node->left)
						rootNode = node->left;
					else
						rootNode = node->right;
					if (rootNode)
						rootNode->parent = NULL;
				}
			}

			node->left = NULL;
			node->right = NULL;
			DeleteNode(node);
			if (retrace)
				Balancing(retrace);
		}
		void Clear(void)
		{
			if (rootNode)
				DeleteNode(rootNode);
			rootNode = NULL;
			count = 0;
		}
		const VALUE* Find(const Key& k) const
		{
			const Node* node = GetNode(k);
			if (node)
				return &node->value;
			return NULL;
		}
		size_t Count(void) const
		{
			return count;
		}
		DKAVLTree& operator = (DKAVLTree&& tree)
		{
			if (this != &tree)
			{
				Clear();

				rootNode = tree.rootNode;
				count = tree.count;
				tree.rootNode = NULL;
				tree.count = 0;
			}
			return *this;
		}
		DKAVLTree& operator = (const DKAVLTree& s)
		{
			if (this == &s)	return *this;

			Clear();

			if (s.rootNode)
				rootNode = s.rootNode->Duplicate();
			count = s.count;
			return *this;
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<VALUE&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](VALUE& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<VALUE&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](VALUE& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
		// lambda enumerator bool (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const VALUE&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const VALUE& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const VALUE&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const VALUE& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
	private:
		class Node
		{
		public:
			Node(const VALUE& v, Node* parentNode)
				: value(v), left(NULL), right(NULL), parent(parentNode), nodeHeight(1), leftHeight(0), rightHeight(0)
			{
			}
			VALUE		value;
			Node*		left;
			Node*		right;
			Node*		parent;
			int			leftHeight;		// left-tree weights
			int			rightHeight;	// right-tree weights
			int			nodeHeight;		// self weights ( = max(left,right)+1)
			Node* Duplicate(void) const
			{
				Node* node = new(Allocator::Alloc(sizeof(Node))) Node(value, NULL);
				if (left)
				{
					node->left = left->Duplicate();
					node->left->parent = node;
				}
				if (right)
				{
					node->right = right->Duplicate();
					node->right->parent = node;
				}
				node->leftHeight = leftHeight;
				node->rightHeight = rightHeight;
				node->nodeHeight = nodeHeight;
				return node;
			}
			template <typename R> bool EnumerateForward(R&& enumerator)
			{
				if (left && left->EnumerateForward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (right && right->EnumerateForward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateBackward(R&& enumerator)
			{
				if (right && right->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (left && left->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateForward(R&& enumerator) const
			{
				if (left && left->EnumerateForward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (right && right->EnumerateForward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateBackward(R&& enumerator) const
			{
				if (right && right->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (left && left->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				return false;
			}
		};
		void DeleteNode(Node* node)
		{
			if (node->right)
				DeleteNode(node->right);
			if (node->left)
				DeleteNode(node->left);

			count--;

			(*node).~Node();
			Allocator::Free(node);
		}
		void LeftRotation(Node* pivot)
		{
			Node* parent = pivot->parent;

			if (parent->parent)
			{
				if (parent->parent->left == parent)
					parent->parent->left = pivot;
				else
					parent->parent->right = pivot;
			}
			pivot->parent = parent->parent;
			parent->parent = pivot;
			parent->right = pivot->left;
			if (parent->right)
				parent->right->parent = parent;
			pivot->left = parent;
		}
		void RightRotation(Node* pivot)
		{
			Node* parent = pivot->parent;

			if (parent->parent)
			{
				if (parent->parent->left == parent)
					parent->parent->left = pivot;
				else
					parent->parent->right = pivot;
			}
			pivot->parent = parent->parent;
			parent->parent = pivot;
			parent->left = pivot->right;
			if (parent->left)
				parent->left->parent = parent;
			pivot->right = parent;
		}
		void UpdateHeight(Node* node)
		{
			node->leftHeight = node->left ? node->left->nodeHeight : 0;
			node->rightHeight = node->right ? node->right->nodeHeight : 0;
			node->nodeHeight = node->leftHeight > node->rightHeight ? node->leftHeight + 1 : node->rightHeight + 1;
		}
		// do balancing tree weights.
		void Balancing(Node* node)
		{
			int left = node->left ? node->left->nodeHeight : 0;
			int right = node->right ? node->right->nodeHeight : 0;

			if (left - right > 1)
			{
				if (node->left->rightHeight > 0 && node->left->rightHeight > node->left->leftHeight)
				{
					// do left-rotate with 'node->left' and right-rotate recursively.
					LeftRotation(node->left->right);
					UpdateHeight(node->left->left);
				}
				// right-rotate with 'node->left'
				RightRotation(node->left);
			}
			else if (right - left > 1)
			{
				if (node->right->leftHeight > 0 && node->right->leftHeight > node->right->rightHeight)
				{
					// right-rotate with 'node->right' and left-rotate recursively.
					RightRotation(node->right->left);
					UpdateHeight(node->right->right);
				}
				// left-rotate with 'node->right'
				LeftRotation(node->right);
			}

			UpdateHeight(node);

			if (node->parent)
				return Balancing(node->parent);
			else
				rootNode = node;
		}
		// find node and return. (create if not exists)
		Node* SetNode(const Value& v, bool* created)
		{
			if (rootNode == NULL)
			{
				*created = true;

				count++;
				rootNode = new(Allocator::Alloc(sizeof(Node))) Node(v, NULL);
				return rootNode;
			}
			Node* node = rootNode;
			while (node)
			{
				int cmp = valueCompareFunc(node->value, v);
				if (cmp > 0)
				{
					if (node->left)
						node = node->left;
					else
					{
						*created = true;
						count++;
						Node* ret = new(Allocator::Alloc(sizeof(Node))) Node(v, node);
						node->left = ret;
						Balancing(node);
						return ret;
					}
				}
				else if (cmp < 0)
				{
					if (node->right)
						node = node->right;
					else
					{
						*created = true;
						count++;
						Node* ret = new(Allocator::Alloc(sizeof(Node))) Node(v, node);
						node->right = ret;
						Balancing(node);
						return ret;
					}
				}
				else
				{
					*created = false;
					return node;
				}
			}
			return NULL;
		}
		// find node 'k' and return. (return NULL if not exists)
		Node* GetNode(const Key& k)
		{
			return const_cast<Node*>(static_cast<const DKAVLTree&>(*this).GetNode(k));
		}
		const Node* GetNode(const Key& k) const
		{
			Node* node = rootNode;
			while (node)
			{
				int cmp = keyCompareFunc(node->value, k);
				if (cmp > 0)
					node = node->left;
				else if (cmp < 0)
					node = node->right;
				else
					return node;
			}
			return NULL;
		}

		Node*			rootNode;
		size_t			count;
		ValueCompare	valueCompareFunc;
		KeyCompare		keyCompareFunc;
		ValueCopy		valueCopyFunc;
	};
}

