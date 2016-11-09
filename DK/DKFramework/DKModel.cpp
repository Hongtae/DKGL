//
//  File: DKModel.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKModel.h"
#include "DKWorld.h"

using namespace DKFramework;

DKModel::DKModel(Type t)
: type(t), parent(NULL), scene(NULL), hideDescendants(false), needResolveTree(true)
{
}

DKModel::DKModel(void)
: DKModel(TypeCustom)
{
}

DKModel::~DKModel(void)
{
	DKASSERT_DEBUG(this->scene == NULL);
	DKASSERT_DEBUG(this->parent == NULL);

	for (DKModel* c : children)
	{
		c->parent = NULL;
		c->OnRemovedFromParent();
		c->needResolveTree = true;
	}
	children.Clear();
}

void DKModel::RemoveFromScene(void)
{
	if (scene)
	{
		if (parent)
			this->RemoveFromParent();

		scene->RemoveObject(this);
	}
}

bool DKModel::CanAcceptObjectAsParent(DKModel*) const
{
	return parent == NULL;
}

bool DKModel::AddChild(DKModel* obj)
{
	if (obj && obj->parent == NULL)
	{
		if (obj->CanAcceptObjectAsParent(this))
		{
			obj->parent = this;
			this->children.Add(obj);
			obj->OnAddedToParent();

			if (this->scene != obj->scene)
			{
				obj->RemoveFromScene();
				if (this->scene)
					this->scene->AddObject(obj);
			}

			this->RootObject()->needResolveTree = true;
			return true;
		}
	}
	return false;
}

void DKModel::RemoveFromParent(void)
{
	if (parent)
	{
		DKObject<DKModel> holder(this);

		DKModel* p = parent;
		for (size_t index = 0; index < p->children.Count(); ++index)
		{
			DKModel* c = p->children.Value(index);
			if (c == this)
			{
				p->children.Remove(index);
				break;
			}
		}

		this->parent = NULL;
		this->OnRemovedFromParent();
		this->needResolveTree = true;
		p->RootObject()->needResolveTree = true;
	}
}

DKModel* DKModel::RootObject(void)
{
	if (parent)
		return parent->RootObject();
	return this;
}

const DKModel* DKModel::RootObject(void) const
{
	if (parent)
		return parent->RootObject();
	return this;
}

bool DKModel::IsDescendantOf(const DKModel* obj) const
{
	if (this == obj)
		return true;
	if (parent)
		return parent->IsDescendantOf(obj);
	return false;
}

size_t DKModel::NumberOfDescendants(void) const
{
	size_t num = 1;
	for (const DKModel* obj : children)
		num += obj->NumberOfDescendants();
	return num;
}

DKModel* DKModel::FindDescendant(const DKString& name)
{
	if (Name() == name)
		return this;
	for (DKModel* obj : children)
	{
		DKModel* p = obj->FindDescendant(name);
		if (p)
			return p;
	}
	return NULL;
}

const DKModel* DKModel::FindDescendant(const DKString& name) const
{
	return const_cast<DKModel&>(*this).FindDescendant(name);
}

bool DKModel::DidAncestorHideDescendants(void) const
{
	for (const DKModel* p = this->parent; p != NULL; p = p->parent)
	{
		if (p->hideDescendants)
			return true;
	}
	return false;
}

bool DKModel::EnumerateInternal(Enumerator* e)
{
	if (e->Invoke(this))
	{
		for (DKModel* c : children)
		{
			if (!c->EnumerateInternal(e))
				return false;
		}
		return true;
	}
	return false;
}

void DKModel::EnumerateInternal(EnumeratorLoop* e)
{
	e->Invoke(this);
	for (DKModel* c : children)
		c->EnumerateInternal(e);
}

bool DKModel::EnumerateInternal(ConstEnumerator* e) const
{
	if (e->Invoke(this))
	{
		for (const DKModel* c : children)
		{
			if (!c->EnumerateInternal(e))
				return false;
		}
		return true;
	}
	return false;
}

void DKModel::EnumerateInternal(ConstEnumeratorLoop* e) const
{
	e->Invoke(this);
	for (const DKModel* c : children)
		c->EnumerateInternal(e);
}

DKModel* DKModel::FindCommonAncestor(DKModel* obj1, DKModel* obj2, Type t)
{
	if (obj1 && obj2)
	{
		if (obj1 == obj2)
			return obj1;

		DKModel* root1 = obj1->RootObject();
		DKModel* root2 = obj2->RootObject();

		if (root1 == root2)
		{
			struct EnumParent
			{
				DKModel* object;
				int maxLevel;
				EnumParent(DKModel* obj)
					: object(obj), maxLevel(0)
				{
					for (DKModel* p = object; p != NULL; p = p->parent)
						maxLevel++;
				}
				DKModel* operator [] (int n)
				{
					DKModel* p = NULL;
					if (n < maxLevel)
					{
						p = object;
						int count = maxLevel - n;
						for (int i = 0; i < count; ++i)
							p = p->parent;
					}
					return p;
				}
			};
			EnumParent ep1(obj1);
			EnumParent ep2(obj2);
			int level = Min(ep1.maxLevel, ep2.maxLevel);
			for (int i = 0; i < level; ++i)
			{
				auto p1 = ep1[i];
				auto p2 = ep2[i];
				if (p1 == p2)
				{
					if (t == TypeCustom || p1->type == t)
						return p1;
				}
			}
		}
	}
	return NULL;
}

void DKModel::SetAnimation(DKAnimatedTransform* anim, bool recursive)
{
	if (this->animation != anim)
	{
		this->animation = anim;
		this->OnSetAnimation(anim);
	}

	if (recursive)
	{
		for (DKModel* obj : children)
			obj->SetAnimation(anim, recursive);
	}
}

void DKModel::SetWorldTransform(const DKNSTransform& t)
{
	worldTransform = t;
	if (parent)
		localTransform = t * DKNSTransform(parent->WorldTransform()).Inverse();
	else
		localTransform = t;
}

void DKModel::SetLocalTransform(const DKNSTransform& t)
{
	localTransform = t;
	if (parent)
		worldTransform = t * parent->WorldTransform();
	else
		worldTransform = t;
}

void DKModel::CreateNamedObjectMap(NamedObjectMap& map)
{
	if (Name().Length() > 0)
		map.Insert(Name(), this);

	for (DKModel* c : children)
		c->CreateNamedObjectMap(map);
}

void DKModel::CreateUUIDObjectMap(UUIDObjectMap& map)
{
	map.Insert(this->UUID(), this);
	for (DKModel* c : children)
		c->CreateUUIDObjectMap(map);
}

void DKModel::ResolveTree(bool force)
{
	struct _Resolver
	{
		DKModel* target;
		_Resolver(DKModel* obj) : target(obj) {}
		void operator ()(NamedObjectMap& map1, UUIDObjectMap& map2)
		{
			target->OnUpdateTreeReferences(map1, map2);
			target->needResolveTree = false;
			for (DKModel* c : target->children)
			{
				_Resolver r(c);
				r(map1, map2);
			}
		}
	};

	if (needResolveTree || force)
	{
		DKModel* root = RootObject();
		NamedObjectMap map1;
		UUIDObjectMap map2;
		root->CreateNamedObjectMap(map1);
		root->CreateUUIDObjectMap(map2);
		_Resolver(this)(map1, map2);
	}
}

void DKModel::ReloadSceneContext(DKOperation* op)
{
	if (scene)
	{
		scene->RemoveSingleObject(this);
		if (op)
			op->Perform();
		scene->AddSingleObject(this);
	}
	else
	{
		if (op)
			op->Perform();
	}
}

void DKModel::OnUpdateKinematic(double timeDelta, DKTimeTick tick)
{
	if (this->animation)
	{
		this->animation->Update(timeDelta, tick);
		DKTransformUnit tu;
		if (this->animation->GetTransform(this->Name(), tu))
		{
			DKNSTransform trans = DKNSTransform(tu.rotation, tu.translation);
			this->SetLocalTransform(trans);
		}
		else
		{
			DKNSTransform trans = this->localTransform;
			this->SetLocalTransform(trans);
		}
	}
}

void DKModel::UpdateKinematic(double timeDelta, DKTimeTick tick)
{
	ResolveTree(false);

	this->OnUpdateKinematic(timeDelta, tick);
	for (DKModel* c : children)
		c->UpdateKinematic(timeDelta, tick);
}

void DKModel::OnUpdateSceneState(const DKNSTransform& t)
{
	this->worldTransform = this->localTransform * t;
}

void DKModel::UpdateSceneState(const DKNSTransform& t)
{
	this->OnUpdateSceneState(t);

	for (DKModel* c : children)
		c->UpdateSceneState(this->worldTransform);
}

void DKModel::UpdateLocalTransform(bool recursive)
{
	DKNSTransform t = worldTransform;
	if (parent)
		t = t * DKNSTransform(parent->WorldTransform()).Inverse();

	this->SetLocalTransform(t);

	if (recursive)
	{
		for (DKModel* c : children)
			c->UpdateLocalTransform(recursive);
	}
}

void DKModel::UpdateWorldTransform(bool recursive)
{
	DKNSTransform t = localTransform;
	if (parent)
		t = t * parent->WorldTransform();

	this->SetWorldTransform(t);

	if (recursive)
	{
		for (DKModel* c : children)
			c->UpdateWorldTransform(recursive);
	}
}

DKObject<DKModel> DKModel::Clone(void) const
{
	UUIDObjectMap uuids;
	DKObject<DKModel> copied = this->Clone(uuids);

	struct _ResolveUUIDs
	{
		DKModel* target;
		_ResolveUUIDs(DKModel* obj) : target(obj) {}
		void operator () (UUIDObjectMap& uuids)
		{
			target->UpdateCopiedReferenceUUIDs(uuids);
			for (DKModel* c : target->children)
			{
				_ResolveUUIDs(c).operator()(uuids);
			}
		}
	};
	_ResolveUUIDs(copied).operator()(uuids);
	return copied;
}

DKObject<DKModel> DKModel::Clone(UUIDObjectMap& uuids) const
{
	DKASSERT_DESC(this->type == TypeCustom, "subclass must override this");

	return DKObject<DKModel>::New()->Copy(uuids, this);
}

DKModel* DKModel::Copy(UUIDObjectMap& uuids, const DKModel* obj)
{
	if (this->parent)
		return NULL;
	if (this->scene)
		return NULL;
	if (this->children.Count() > 0)
		return NULL;

	if (obj && uuids.Insert(obj->UUID(), this))
	{
		this->SetName(obj->Name());
		this->localTransform = obj->localTransform;
		this->worldTransform = obj->worldTransform;

		for (const DKModel* m : obj->children)
		{
			DKObject<DKModel> c = m->Clone(uuids);
			if (c == NULL)
			{
				return NULL;
			}
			if (this->AddChild(c) == false)
			{
				return NULL;
			}
		}

		return this;
	}
	return NULL;
}

DKObject<DKSerializer> DKModel::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		bool resolveTree;
		DKObject<DKModel> target;

		// localTransform  
		void GetLocalTransform(DKVariant& v) const
		{
			const DKNSTransform& t = target->localTransform;
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"orientation", (const DKVariant::VQuaternion&)t.orientation);
			v.Pairs().Insert(L"position", (const DKVariant::VVector3&)t.position);
		}
		void SetLocalTransform(DKVariant& v)
		{
			const DKVariant::VPairs::Pair* orientation = v.Pairs().Find(L"orientation");
			const DKVariant::VPairs::Pair* position = v.Pairs().Find(L"position");
			if (orientation && orientation->value.ValueType() == DKVariant::TypeQuaternion &&
				position && position->value.ValueType() == DKVariant::TypeVector3)
			{
				target->localTransform = DKNSTransform(orientation->value.Quaternion(), position->value.Vector3());
			}
			else
			{
				target->localTransform.Identity();
			}
		}
		bool CheckLocalTransform(const DKVariant& v)
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		void ResetLocalTransform(void)
		{
			target->localTransform.Identity();
		}
		// children
		void GetChildren(ExternalArrayType& v)
		{
			v.Reserve(target->children.Count());
			for (DKModel* obj : target->children)
				v.Add(obj);
		}
		void SetChildren(ExternalArrayType& v)
		{
			target->children.Clear();
			target->children.Reserve(v.Count());
			for (DKObject<DKResource>& res : v)
			{
				DKModel* obj = res.SafeCast<DKModel>();
				if (obj)
					target->AddChild(obj);
			}
		}
		// callback  
		void Callback(State s)
		{
			if (s == StateSerializeBegin)
			{
				this->resolveTree = target->Parent() == NULL;
			}
			else if (s == StateDeserializeBegin)
			{
				this->resolveTree = false;
			}
			else if (s == StateDeserializeSucceed)
			{
				if (this->resolveTree && target->Parent() == NULL)
				{
					target->ResolveTree(true);
					target->UpdateWorldTransform();
				}
			}
		}
		DKSerializer* Init(DKModel* p)
		{
			if (p == NULL)
				return NULL;

			this->target = p;
			this->SetResourceClass(L"DKModel");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"localTransform",
				DKFunction(this, &LocalSerializer::GetLocalTransform),
				DKFunction(this, &LocalSerializer::SetLocalTransform),
				DKFunction(this, &LocalSerializer::CheckLocalTransform),
				DKFunction(this, &LocalSerializer::ResetLocalTransform)->Invocation());

			this->Bind(L"children",
				DKFunction(this, &LocalSerializer::GetChildren),
				DKFunction(this, &LocalSerializer::SetChildren),
				NULL,
				ExternalResourceInclude,
				DKFunction([p]{ p->children.Clear(); })->Invocation());

			if (p->Parent() == NULL)
			{
				this->Bind(L"resolveTree",
					DKFunction([this](DKVariant& v) {v.SetInteger(this->resolveTree); }),
					DKFunction([this](DKVariant& v) {this->resolveTree = v.Integer() != 0; }),
					DKFunction([](const DKVariant& v)->bool {return v.ValueType() == DKVariant::TypeInteger; }),
					DKFunction([this] {this->resolveTree = false; })->Invocation());
			}

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
