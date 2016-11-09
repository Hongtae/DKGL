//
//  File: DKAVLTree.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
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
// perform re-balance when both nodes weights diff > 1
//
// Note:
//  value's pointer will not be changed after balancing process.
//  You can save pointer if you wish.
//
//  This class is not thread-safe. You need to use synchronization object
//  to serialize of access in multi-threaded environment.
//  You can use DKMap, DKSet instead, they are thread safe.
//

namespace DKFoundation
{
	template <typename Value, typename Key> struct DKTreeItemComparator
	{
		FORCEINLINE int operator () (const Value& lhs, const Key& rhs) const
		{
			if (lhs > rhs)				return 1;
			else if (lhs < rhs)			return -1;
			return 0;
		}
	};
	template <typename Value> struct DKTreeItemReplacer
	{
		FORCEINLINE void operator () (Value& dst, const Value& src) const
		{
			dst = src;
		}
	};

	template <
		typename Value,												// value-type
		typename Comparator = DKTreeItemComparator<Value, Value>,	// value comparison
		typename Replacer = DKTreeItemReplacer<Value>,				// value replacement
		typename Allocator = DKMemoryDefaultAllocator				// memory allocator
	>
	class DKAVLTree
	{
		struct Node
		{
			Node(const Value& v) : value(v), left(NULL), right(NULL), leftHeight(0), rightHeight(0) {}

			Value		value;
			Node*		left;
			Node*		right;
			uint16_t	leftHeight;		// left-tree weights
			uint16_t	rightHeight;	// right-tree weights

			FORCEINLINE uint16_t Height(void) const
			{
				return leftHeight > rightHeight ? (leftHeight + 1) : (rightHeight + 1);
			}
			Node* Duplicate(void) const
			{
				Node* node = new(Allocator::Alloc(sizeof(Node))) Node(value);
				if (left)
				{
					node->left = left->Duplicate();
				}
				if (right)
				{
					node->right = right->Duplicate();
				}
				node->leftHeight = leftHeight;
				node->rightHeight = rightHeight;
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

	public:
		using ValueTraits = DKTypeTraits<Value>;

		Comparator		comparator;
		Replacer		replacer;

		constexpr static size_t NodeSize(void)	{ return sizeof(Node); }

		DKAVLTree(void)
			: rootNode(NULL), count(0)
		{
		}
		DKAVLTree(DKAVLTree&& tree)
			: rootNode(NULL), count(0)
			, comparator(static_cast<Comparator&&>(tree.comparator))
			, replacer(static_cast<Replacer&&>(tree.replacer))
		{
			rootNode = tree.rootNode;
			count = tree.count;
			tree.rootNode = NULL;
			tree.count = 0;
		}
		// Copy constructor. accepts same type of class.
		// templates not works on MSVC (bug?)
		DKAVLTree(const DKAVLTree& tree)
			: rootNode(NULL), count(0)
			, comparator(tree.comparator)
			, replacer(tree.replacer)
		{
			if (tree.rootNode)
				rootNode = tree.rootNode->Duplicate();
			count = tree.count;
		}
		~DKAVLTree(void)
		{
			Clear();
		}
		// Update: insertion if not exist or overwrite if exists.
		FORCEINLINE const Value* Update(const Value& v)
		{
			if (rootNode)
			{
				LocationContext ctxt = {&v};
				LocateNodeForValue(rootNode, &ctxt);
				if (ctxt.balancedNode)
					rootNode = ctxt.balancedNode;
				else
					replacer(ctxt.locatedNode->value, v);
				return &(ctxt.locatedNode->value);
			}
			count = 1;
			rootNode = new(Allocator::Alloc(sizeof(Node))) Node(v);
			return &(rootNode->value);
		}
		// Insert: insert if not exist or fail if exists.
		//  returns NULL if function failed. (already exists)
		FORCEINLINE const Value* Insert(const Value& v)
		{
			if (rootNode)
			{
				size_t c = this->count;
				LocationContext ctxt = {&v};
				LocateNodeForValue(rootNode, &ctxt);
				if (ctxt.balancedNode)
					rootNode = ctxt.balancedNode;

				if (this->count != c)	// new item.
					return &(ctxt.locatedNode->value);
				return NULL;
			}
			count = 1;
			rootNode = new(Allocator::Alloc(sizeof(Node))) Node(v);
			return &(rootNode->value);
		}
		template <typename Key, typename KeyValueComparator>
		FORCEINLINE void Remove(const Key& k, KeyValueComparator&& comp)
		{
			if (rootNode)
			{
				LocationContext ctxt = { NULL, NULL, NULL };
				TakeOutNodeForKey(rootNode, k, std::forward<KeyValueComparator>(comp), &ctxt);
				Node* node = ctxt.locatedNode;
				if (node)
				{
					rootNode = ctxt.balancedNode;
					DeleteNode(node);
				}
			}
		}
		FORCEINLINE void Clear(void)
		{
			if (rootNode)
				DeleteNode(rootNode);
			rootNode = NULL;
			count = 0;
		}
		template <typename Key, typename KeyValueComparator>
		FORCEINLINE const Value* Find(const Key& k, KeyValueComparator&& cmp) const
		{
			const Node* node = LookupNodeForKey(k, std::forward<KeyValueComparator>(cmp));
			if (node)
				return &node->value;
			return NULL;
		}
		FORCEINLINE size_t Count(void) const
		{
			return count;
		}
		DKAVLTree& operator = (DKAVLTree&& tree)
		{
			if (this != &tree)
			{
				Clear();

				comparator = static_cast<Comparator&&>(tree.comparator);
				replacer = static_cast<Replacer&&>(tree.replacer);

				rootNode = tree.rootNode;
				count = tree.count;
				tree.rootNode = NULL;
				tree.count = 0;
			}
			return *this;
		}
		DKAVLTree& operator = (const DKAVLTree& tree)
		{
			if (this == &tree)	return *this;

			Clear();

			if (tree.rootNode)
				rootNode = tree.rootNode->Duplicate();
			count = tree.count;
			comparator = tree.comparator;
			replacer = tree.replacer;
			return *this;
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<Value&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<Value&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
		// lambda enumerator bool (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const Value&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const Value&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
		
	private:
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
		FORCEINLINE Node* LeftRotate(Node* node)
		{
			Node* right = node->right;
			node->right = right->left;
			right->left = node;
			return right;
		}
		FORCEINLINE Node* RightRotate(Node* node)
		{
			Node* left = node->left;
			node->left = left->right;
			left->right = node;
			return left;
		}
		FORCEINLINE void UpdateHeight(Node* node)
		{
			node->leftHeight = node->left ? node->left->Height() : 0;
			node->rightHeight = node->right ? node->right->Height() : 0;
		}
		// balance tree weights.
		FORCEINLINE Node* Balance(Node* node)
		{
			Node* node2 = node;
			int left = node->left ? node->left->Height() : 0;
			int right = node->right ? node->right->Height() : 0;

			int d = left - right;
			if (d > 1)
			{
				if (node->left->rightHeight > 0 && node->left->rightHeight > node->left->leftHeight)
				{
					// do left-rotate with 'node->left' and right-rotate recursively.
					node->left = LeftRotate(node->left);
					UpdateHeight(node->left->left);
				}
				// right-rotate with 'node' and 'node->left'
				node2 = RightRotate(node);
			}
			else if (d < -1)
			{
				if (node->right->leftHeight > 0 && node->right->leftHeight > node->right->rightHeight)
				{
					// right-rotate with 'node->right' and left-rotate recursively.
					node->right = RightRotate(node->right);
					UpdateHeight(node->right->right);
				}
				// left-rotate with 'node' and 'node->right'
				node2 = LeftRotate(node);
			}
			UpdateHeight(node);
			if (node != node2)
				UpdateHeight(node2);
			return node2;
		}
		struct LocationContext
		{
			const Value* value;
			Node* locatedNode;
			Node* balancedNode;	// valid only if tree needs to be balanced.
			int cmp;
		};
		void TakeOutLeftMostNode(Node* node, LocationContext* ctxt)
		{
			if (node->left)
			{
				TakeOutLeftMostNode(node->left, ctxt);
				node->left = ctxt->balancedNode;
				ctxt->balancedNode = Balance(node);
			}
			else
			{
				ctxt->locatedNode = node;
				ctxt->balancedNode = node->right;
				node->right = NULL;
			}
		}
		void TakeOutRightMostNode(Node* node, LocationContext* ctxt)
		{
			if (node->right)
			{
				TakeOutRightMostNode(node->right, ctxt);
				node->right = ctxt->balancedNode;
				ctxt->balancedNode = Balance(node);
			}
			else
			{
				ctxt->locatedNode = node;
				ctxt->balancedNode = node->left;
				node->left = NULL;
			}
		}
		// find item and take out from tree.
		template <typename Key, typename KeyComparator>
		void TakeOutNodeForKey(Node* node, const Key& key, KeyComparator&& comp, LocationContext* ctxt)
		{
			ctxt->cmp = comp(node->value, key);
			if (ctxt->cmp > 0)
			{
				if (node->left)
				{
					TakeOutNodeForKey(node->left, key, std::forward<KeyComparator>(comp), ctxt);
					if (ctxt->locatedNode)
					{
						node->left = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
			}
			else if (ctxt->cmp < 0)
			{
				if (node->right)
				{
					TakeOutNodeForKey(node->right, key, std::forward<KeyComparator>(comp), ctxt);
					if (ctxt->locatedNode)
					{
						node->right = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
			}
			else
			{
				if (node->left && node->right)
				{
					if (node->leftHeight > node->rightHeight)
					{
						TakeOutRightMostNode(node->left, ctxt);
						ctxt->locatedNode->left = ctxt->balancedNode;
						ctxt->locatedNode->right = node->right;
					}
					else
					{
						TakeOutLeftMostNode(node->right, ctxt);
						ctxt->locatedNode->right = ctxt->balancedNode;
						ctxt->locatedNode->left = node->left;
					}
					node->left = NULL;
					node->right = NULL;
					ctxt->balancedNode = Balance(ctxt->locatedNode);
					ctxt->locatedNode = node;
				}
				else
				{
					ctxt->locatedNode = node;
					if (node->left)
						ctxt->balancedNode = node->left;
					else
						ctxt->balancedNode = node->right;
					node->left = NULL;
					node->right = NULL;
				}
			}
		}
		// locate node for value. (create if not exists)
		// this function uses 'LocationContext' value instead of
		// stack variables, because of called recursively.
		void LocateNodeForValue(Node* node, LocationContext* ctxt)
		{
			ctxt->cmp = comparator(node->value, *ctxt->value);
			if (ctxt->cmp > 0)
			{
				if (node->left)
				{
					LocateNodeForValue(node->left, ctxt);
					if (ctxt->balancedNode)
					{
						node->left = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
				else
				{
					node->left = new(Allocator::Alloc(sizeof(Node))) Node(*ctxt->value);
					node->leftHeight = 1;
					ctxt->locatedNode = node->left;
					ctxt->balancedNode = node->right ? NULL : node;
					count++;
					return;
				}
			}
			else if (ctxt->cmp < 0)
			{
				if (node->right)
				{
					LocateNodeForValue(node->right, ctxt);
					if (ctxt->balancedNode)
					{
						node->right = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
				else
				{
					node->right = new(Allocator::Alloc(sizeof(Node))) Node(*ctxt->value);
					node->rightHeight = 1;
					ctxt->locatedNode = node->right;
					ctxt->balancedNode = node->left ? NULL : node;
					count++;
					return;
				}
			}
			else
			{
				ctxt->locatedNode = node;
				ctxt->balancedNode = NULL;
			}
		}
		template <typename Key, typename KeyComparator>
		FORCEINLINE const Node* LookupNodeForKey(const Key& k, KeyComparator&& comp) const
		{
			Node* node = rootNode;
			while (node)
			{
				int d = comp(node->value, k);
				if (d > 0)
					node = node->left;
				else if (d < 0)
					node = node->right;
				else
					return node;
			}
			return NULL;
		}

		Node*			rootNode;
		size_t			count;
	};
}
