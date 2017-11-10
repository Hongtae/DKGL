//
//  File: DKAnimation.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKAnimation.h"

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			inline DKVector3 Interpolate(const DKVector3& v1, const DKVector3& v2, float t)
			{
				if (t <= 0)
					return v1;
				else if (t >= 1)
					return v2;
				return v1 + ((v2 - v1) * t);
			}
			inline DKQuaternion Interpolate(const DKQuaternion& q1, const DKQuaternion& q2, float t)
			{
				if (t <= 0)
					return q1;
				else if (t >= 1)
					return q2;
				return DKQuaternion::Slerp(q1, q2, t);
			}
			template <typename T> inline T Interpolate(const T& key1, const T& key2, float t)	// T = KeyframeNode::TranslationKey, KeyFrameNode::RotationKey, KeyFrameNode::ScaleKey
			{
				float interval = key2.time - key1.time;
				float delta = t - key1.time;

				T ret = {t, Interpolate(key1.key, key2.key, delta / interval)};
				return ret;
			}
			template <typename T> inline bool SortKeyframeByTime(const T& lhs, const T& rhs) // sort by time asc
			{
				return lhs.time < rhs.time;
			}
			// Clipping key frame with time scale ( 0 <= frame <= 1 ) and sort by time ascending.
			// first, last key frame will be interpolated.
			template <typename T> inline DKArray<T> ClipKeyframeTimeScale(const T* frames, size_t numFrames)
			{
				DKArray<T> output;
				output.Reserve(numFrames);

				const T* beforeBeginning = NULL; // key frame just before the first. (biggest and smaller than 0.0)
				const T* beginning = NULL;       // first frame (greater or equals to 0.0)
				const T* afterEnding = NULL;     // key frame just after the last. (smallest and bigger than 1.0)
				const T* ending = NULL;          // last frame (smaller or equals to 1.0)

				for (size_t i = 0; i < numFrames; ++i)
				{
					const T& f = frames[i];

					if (f.time < 0.0f)
					{
						// finding nearest to 0.0 from all frames smaller than 0.0
						if (beforeBeginning == NULL || beforeBeginning->time < f.time)
							beforeBeginning = &f;
					}
					else if (f.time > 1.0f)
					{
						// finding nearest to 1.0 from all frames bigger than 1.0
						if (afterEnding == NULL || afterEnding->time < f.time)
							afterEnding = &f;
					}
					else		// 0 <= f.time <= 1
					{
						if (beginning == NULL || beginning->time > f.time)
							beginning = &f;
						if (ending == NULL || ending->time < f.time)
							ending = &f;

						output.Add(f);
					}
				}

				// generate interpolated first key frame with time = minT
				if (beforeBeginning && beginning && beginning->time > 0.0f)
				{
					T frm = Interpolate(*beforeBeginning, *beginning, 0.0f);
					output.Add(frm);
				}
				// generate interpolated last key frame with time = maxT
				if (afterEnding && ending && ending->time < 1.0f)
				{
					T frm = Interpolate(*ending, *afterEnding, 1.0f);
					output.Add(frm);
				}

				// sort by time ascending.
				output.Sort(0, output.Count(), SortKeyframeByTime<T>);
				return output;
			}
			template <typename T> inline DKArray<T> ClipKeyframeTimeScale(const DKArray<T>& frame)		
			{
				return ClipKeyframeTimeScale((const T*)frame, frame.Count());
			}
			// finding nearest two key frames (before the first, after the last)
			// with (begin <= t < end)
			template <typename T> inline bool FindNearKeyFrames(const DKArray<T>& frames, size_t begin, size_t count, T& prev, T& next, float time)
			{
				while (count > 2)
				{
					size_t middle = begin + count / 2;
					if (frames.Value(middle).time < time)
					{
						count = begin + count - middle;
						begin = middle;
					}
					else
					{
						count = middle - begin + 1;
					}
				}
				if (count > 1)
				{
					prev = frames.Value(begin);
					next = frames.Value(begin+1);
					return true;
				}
				return false;
			}
			template <typename T> inline bool FindNearKeyFrames(const DKArray<T>& frames, T& prev, T& next, float time)
			{
				return FindNearKeyFrames(frames, 0, frames.Count(), prev, next, time);
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;


DKAnimation::DKAnimation(void)
	: duration(0)
{
}

DKAnimation::~DKAnimation(void)
{
	RemoveAllNodes();
}

bool DKAnimation::GetNodeTransform(NodeIndex index, float t, DKTransformUnit& output) const
{
	const Node* node = NodeAtIndex(index);
	if (node)
	{
		output = GetTransform(*node, t);
		return true;
	}
	return false;
}

bool DKAnimation::GetNodeTransform(const DKString& name, float t, DKTransformUnit& output) const
{
	return GetNodeTransform(IndexOfNode(name), t, output);
}

void DKAnimation::SetDuration(float d)
{
	if (d > 0)
		duration = d;
	else
		duration = 0;
}

float DKAnimation::Duration(void) const
{
	return duration;
}

DKArray<DKAnimation::NodeSnapshot> DKAnimation::CreateSnapshot(float t) const
{
	DKArray<NodeSnapshot> result;
	result.Reserve(nodes.Count());
	for (size_t i = 0; i < nodes.Count(); ++i)
	{
		NodeSnapshot ns;
		const Node* node = nodes.Value(i);
		ns.name = node->name;
		ns.transform = GetTransform(*node, t);
		result.Add(ns);
	}
	return result;
}

DKObject<DKAnimation> DKAnimation::Create(DKArray<SamplingNode>* samples, DKArray<KeyframeNode>* keyframes, float duration)
{
	DKObject<DKAnimation> ani = DKObject<DKAnimation>::New();

	for (int i = 0; i < samples->Count(); i++)
		ani->AddNode(&(samples->Value(i)));

	for (int i = 0; i < keyframes->Count(); i++)
		ani->AddNode(&(keyframes->Value(i)));

	ani->SetDuration(duration);
	return ani;
}

DKObject<DKAnimation> DKAnimation::Create(DKArray<NodeSnapshot>* begin, DKArray<NodeSnapshot>* end, float duration)
{
	DKObject<DKAnimation> ani = DKObject<DKAnimation>::New();

	for (int i = 0; i < begin->Count(); i++)
	{
		NodeSnapshot& s = begin->Value(i);
		if (s.name.Length() > 0)
		{
			SamplingNode	node;
			node.name = s.name;
			node.frames.Add(s.transform);
			if (end)
			{
				for (int k = 0; k < end->Count(); k++)
				{
					NodeSnapshot& e = end->Value(k);
					if (e.name == s.name)
					{
						node.frames.Add(e.transform);
						break;
					}
				}
			}
			ani->AddNode(&node);
		}
	}
	ani->SetDuration(duration);
	return ani;
}

bool DKAnimation::AddNode(const Node* node)
{
	if (node == NULL)
		return false;

	if (nodeIndexMap.Find(node->name))
		return false;

	if (node->IsEmpty())
		return false;

	if (node->type == Node::NodeTypeSampling)
	{
		const SamplingNode* s = static_cast<const SamplingNode*>(node);
		return this->AddSamplingNode(s->name, s->frames, s->frames.Count());
	}
	else if (node->type == Node::NodeTypeKeyframe)
	{
		const KeyframeNode* k = static_cast<const KeyframeNode*>(node);
		return this->AddKeyframeNode(k->name,
									 k->scaleKeys, k->scaleKeys.Count(),
									 k->rotationKeys, k->rotationKeys.Count(),
									 k->translationKeys, k->translationKeys.Count());
	}

	return false;
}

bool DKAnimation::AddSamplingNode(const DKString& name, const DKTransformUnit* frames, size_t numFrames)
{
	if (nodeIndexMap.Find(name))
		return false;
	if (frames && numFrames > 0)
	{
		SamplingNode* node = DKRawPtrNew<SamplingNode>();
		node->name = name;
		node->frames.Add(frames, numFrames);
		nodeIndexMap.Update(node->name, nodes.Add(node)); // add new node, and update indexes.
		return true;
	}
	return false;
}

bool DKAnimation::AddKeyframeNode(const DKString& name,
								  const KeyframeNode::ScaleKey* scaleKeys, size_t numSk,
								  const KeyframeNode::RotationKey* rotationKeys, size_t numRk,
								  const KeyframeNode::TranslationKey* translationKeys, size_t numTk)
{
	if (nodeIndexMap.Find(name))
		return false;
	if ((scaleKeys && numSk > 0) || (rotationKeys && numRk > 0) || (translationKeys && numTk > 0))
	{
		KeyframeNode* node = DKRawPtrNew<KeyframeNode>();
		node->name = name;
		 // filter by time in scale of 0.0 ~ 1.0, and sort, copy frames.
		node->scaleKeys = ClipKeyframeTimeScale(scaleKeys, numSk);

		node->rotationKeys = ClipKeyframeTimeScale(rotationKeys, numRk);
		node->translationKeys = ClipKeyframeTimeScale(translationKeys, numTk);

		if (!node->IsEmpty())
		{
			nodeIndexMap.Update(node->name, nodes.Add(node));
			return true;
		}
		node->~KeyframeNode();
		DKFree(node);
	}
	return false;
}

void DKAnimation::RemoveNode(const DKString& name)
{
	DKMap<DKString, size_t>::Pair* indexPtr = nodeIndexMap.Find(name);
	if (indexPtr)
	{
		size_t index = indexPtr->value;
		Node* n = nodes.Value(index);
		nodes.Remove(index);
		DKRawPtrDelete(n);
	}
	nodeIndexMap.Remove(name);
}

void DKAnimation::RemoveAllNodes(void)
{
	for (size_t i = 0; i < nodes.Count(); ++i)
	{
		Node* n = nodes.Value(i);
		DKRawPtrDelete(n);
	}
	nodes.Clear();
	nodeIndexMap.Clear();
}

size_t DKAnimation::NodeCount(void) const
{
	return nodes.Count();
}

DKAnimation::NodeIndex DKAnimation::IndexOfNode(const DKString& name) const
{
	const DKMap<DKString, size_t>::Pair* indexPtr = nodeIndexMap.Find(name);
	if (indexPtr)
		return indexPtr->value;
	return invalidNodeIndex;
}

const DKAnimation::Node* DKAnimation::NodeAtIndex(NodeIndex index) const
{
	if (index >= 0 && index < nodes.Count())
	{
		return nodes[index];
	}
	return NULL;
}

DKTransformUnit DKAnimation::GetTransform(const Node& node, float time)
{
	DKTransformUnit output;
	output.Identity();

	if (node.type == Node::NodeTypeSampling)
	{
		const DKArray<DKTransformUnit>& samples = ((const SamplingNode&)node).frames;
		size_t count = samples.Count();
		if (count > 0)
		{
			if (time <= 0 || count == 1)
			{
				output = samples.Value(0);
			}
			else if (time >= 1.0f)
			{
				output = samples[count - 1];
			}
			else
			{
				float elapsed = ((float)count - 1) * time;
				int index = (int)elapsed;
				float interval = elapsed - index;

				const DKTransformUnit& t1 = samples.Value(index);
				const DKTransformUnit& t2 = samples.Value(index+1);

				output = t1.Interpolate(t2, interval);
			}
		}
	}
	else if (node.type == Node::NodeTypeKeyframe)
	{
		const DKArray<KeyframeNode::TranslationKey>& translationKeys = ((const KeyframeNode&)node).translationKeys;
		const DKArray<KeyframeNode::RotationKey>& rotationKeys = ((const KeyframeNode&)node).rotationKeys;
		const DKArray<KeyframeNode::ScaleKey>& scaleKeys = ((const KeyframeNode&)node).scaleKeys;

		if (translationKeys.IsEmpty())
			output.translation = DKVector3(0,0,0);
		else
		{
			KeyframeNode::TranslationKey translate1, translate2;
			if (FindNearKeyFrames(translationKeys, translate1, translate2, time))
				output.translation = Interpolate(translate1.key, translate2.key, (time - translate1.time) / (translate2.time - translate1.time));
			else
				output.translation = translationKeys.Value(0).key;
		}

		if (rotationKeys.IsEmpty())
			output.rotation.Identity();
		else
		{
			KeyframeNode::RotationKey rotate1, rotate2;
			if (FindNearKeyFrames(rotationKeys, rotate1, rotate2, time))
				output.rotation = Interpolate(rotate1.key, rotate2.key, (time - rotate1.time) / (rotate2.time - rotate1.time));
			else
				output.rotation = rotationKeys.Value(0).key;
		}

		if (scaleKeys.IsEmpty())
			output.scale = DKVector3(1,1,1);
		else
		{
			KeyframeNode::ScaleKey scale1, scale2;
			if (FindNearKeyFrames(scaleKeys, scale1, scale2, time))
				output.scale = Interpolate(scale1.key, scale2.key, (time - scale1.time) / (scale2.time - scale1.time));
			else
				output.scale = scaleKeys.Value(0).key;
		}
	}
	return output;
}

bool DKAnimation::ResampleNode(const Node& source, unsigned int frames, KeyframeNode& output, float threshold)
{
	if (source.IsEmpty())
		return false;

	const Node* samplingTarget = &source;
	KeyframeNode tmp;

	// sort by time, filter by time in scale of 0.0 ~ 1.0, if target is key-frame.
	if (samplingTarget->type == Node::NodeTypeKeyframe)
	{
		tmp.translationKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->translationKeys);
		tmp.rotationKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->rotationKeys);
		tmp.scaleKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->scaleKeys);
		samplingTarget = &tmp;
	}

	// using epsilon a bit greater than FLT_EPSILON.
	// if epsilon is bigger, will lost detail actions. (lost frames)
	// if epsilon is smaller, size will bigger. (too many frames)
	float positionThreshold = Max(threshold, 0.000001f);	// accuracy epsilon for position
	float rotationThreshold = Max(threshold, 0.000001f);	// accuracy epsilon for rotate (-1.0 ~ 1.0)
	float scaleThreshold = Max(threshold, 0.000001f);		// accuracy epsilon for scale

	output.name = source.name;
	output.translationKeys.Clear();
	output.rotationKeys.Clear();
	output.scaleKeys.Clear();
	output.translationKeys.Reserve(frames);
	output.rotationKeys.Reserve(frames);
	output.scaleKeys.Reserve(frames);

	// insert first frame with time=0.0 explicitly.
	{
		DKTransformUnit transform = GetTransform(*samplingTarget, 0.0);
		KeyframeNode::TranslationKey t = {0, transform.translation};
		KeyframeNode::RotationKey r = {0, transform.rotation};
		KeyframeNode::ScaleKey s = {0, transform.scale};
		output.translationKeys.Add(t);
		output.rotationKeys.Add(r);
		output.scaleKeys.Add(s);
	}
	// calculate displacement of frames (without first, last)
    // and insert interpolated key frame.
	for (unsigned int i = 1; i < frames - 1; i++)
	{
		float currentTime = static_cast<float>(i) / static_cast<float>(frames-1);
		float nextTime = static_cast<float>(i+1) / static_cast<float>(frames-1);

		DKTransformUnit current = GetTransform(*samplingTarget, currentTime);
		DKTransformUnit next = GetTransform(*samplingTarget, nextTime);

		KeyframeNode::TranslationKey& lastT = output.translationKeys.Value(output.translationKeys.Count()-1);
		KeyframeNode::RotationKey& lastR = output.rotationKeys.Value(output.rotationKeys.Count()-1);
		KeyframeNode::ScaleKey& lastS = output.scaleKeys.Value(output.scaleKeys.Count()-1);

		DKVector3 translate = Interpolate(lastT.key, next.translation, (currentTime - lastT.time) / (nextTime - lastT.time));
		DKQuaternion rotate = Interpolate(lastR.key, next.rotation, (currentTime - lastR.time) / (nextTime - lastR.time));
		DKVector3 scale = Interpolate(lastS.key, next.scale, (currentTime - lastS.time) / (nextTime - lastS.time));

		DKVector3 translateDelta = current.translation - translate;
		if (fabs(translateDelta.x) > positionThreshold || fabs(translateDelta.y) > positionThreshold || fabs(translateDelta.z) > positionThreshold)
		{
			KeyframeNode::TranslationKey t = {currentTime, current.translation};
			output.translationKeys.Add(t);
		}
		float rotateDelta = fabs(DKQuaternion::Dot(current.rotation, rotate));
		if (1.0 - rotateDelta > rotationThreshold)
		{
			KeyframeNode::RotationKey r = {currentTime, current.rotation};
			output.rotationKeys.Add(r);
		}
		DKVector3 scaleDelta = current.scale - scale;
		if (fabs(scaleDelta.x) > scaleThreshold || fabs(scaleDelta.y) > scaleThreshold || fabs(scaleDelta.z) > scaleThreshold)
		{
			KeyframeNode::ScaleKey s = {currentTime, current.scale};
			output.scaleKeys.Add(s);
		}
	}
	// insert last frame with time=1.0 explicitly.
	{
		DKTransformUnit transform = GetTransform(*samplingTarget, 1.0);
		KeyframeNode::TranslationKey t = {1.0, transform.translation};
		KeyframeNode::RotationKey r = {1.0, transform.rotation};
		KeyframeNode::ScaleKey s = {1.0, transform.scale};
		output.translationKeys.Add(t);
		output.rotationKeys.Add(r);
		output.scaleKeys.Add(s);
	}
	return true;
}

bool DKAnimation::ResampleNode(const Node& source, unsigned int frames, SamplingNode& output)
{
	if (source.IsEmpty())
		return false;

	const Node* samplingTarget = &source;
	KeyframeNode tmp;
	// if type is keyframe, sort by time and filter by time 0.0~1.0
	if (samplingTarget->type == Node::NodeTypeKeyframe)
	{
		tmp.translationKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->translationKeys);
		tmp.rotationKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->rotationKeys);
		tmp.scaleKeys = ClipKeyframeTimeScale(static_cast<const KeyframeNode*>(samplingTarget)->scaleKeys);
		samplingTarget = &tmp;
	}

	output.name = source.name;
	output.frames.Clear();
	output.frames.Reserve(frames);

	for (unsigned int i = 0; i < frames; i++)
	{
		output.frames.Add(GetTransform(*samplingTarget, double(i)/double(frames)));
	}

	return true;
}

DKObject<DKAnimationController> DKAnimation::CreateLoopController(void)
{
	struct Controller : public DKAnimationController
	{
		DKObject<DKAnimation> animation;
		float frameTime;
		bool playing;

		void UpdateFrame(float frame)
		{
			if (animation)
			{
				float duration = animation->Duration();
				frameTime = frame / duration;
				if (frameTime > 1.0 || frameTime < 0.0)
					frameTime -= floor(frameTime);
			}
		}
		bool GetTransform(const NodeId& key, DKTransformUnit& out)
		{
			if (animation)
				return animation->GetNodeTransform(key, frameTime, out);
			return false;
		}
		bool IsPlaying(void) const
		{
			return playing;
		}
		float Duration(void) const
		{
			if (animation)
				return animation->Duration();
			return 0.0f;
		}
		void Play(void)
		{
			if (!playing && animation)
				playing = true;
		}
		void Stop(void)
		{
			playing = false;
		}
	};

	DKObject<Controller> con = DKObject<Controller>::New();
	con->animation = this;
	con->frameTime = 0;
	con->playing = false;

	return con.SafeCast<DKAnimationController>();
}

DKObject<DKSerializer> DKAnimation::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKAnimation* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKAnimation");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKResource::Serializer(), NULL);
			this->Bind(L"duration",
				DKFunction(this, &LocalSerializer::GetDuration),
				DKFunction(this, &LocalSerializer::SetDuration),
				DKFunction(this, &LocalSerializer::CheckDuration),
				NULL);
			this->Bind(L"samplingNodes",
				DKFunction(this, &LocalSerializer::GetSamplingNode),
				DKFunction(this, &LocalSerializer::SetSamplingNode),
				DKFunction(this, &LocalSerializer::CheckSamplingNode),
				NULL);
			this->Bind(L"keyframeNodes",
				DKFunction(this, &LocalSerializer::GetKeyframeNode),
				DKFunction(this, &LocalSerializer::SetKeyframeNode),
				DKFunction(this, &LocalSerializer::CheckKeyframeNode),
				NULL);

			return this;
		}
	private:
		void GetSamplingNodeVariant(DKVariant& v, const SamplingNode& sn) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"name", (const DKVariant::VString&)sn.name);

			DKVariant frames(DKVariant::TypeStructData);
			DKVariant::VStructuredData& data = frames.StructuredData();
			data.data = DKOBJECT_NEW DKBuffer((const DKTransformUnit*)sn.frames, sn.frames.Count() * sizeof(DKTransformUnit));
			data.elementSize = sizeof(DKTransformUnit);
			data.layout.Add(DKVariant::StructElem::Arithmetic4, sizeof(DKTransformUnit) / 4);
			v.Pairs().Insert(L"frames", frames);
		}
		bool SetSamplingNodeVariant(const DKVariant& v, SamplingNode& sn) const
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				const DKVariant::VPairs::Pair* pName = v.Pairs().Find(L"name");
				const DKVariant::VPairs::Pair* pFrames = v.Pairs().Find(L"frames");

				if (pName && pName->value.ValueType() == DKVariant::TypeString && pFrames)
				{
					const DKData* frameData = NULL;
					if (pFrames->value.ValueType() == DKVariant::TypeData)
					{
						frameData = &(pFrames->value.Data());
					}
					else if (pFrames->value.ValueType() == DKVariant::TypeStructData)
					{
						frameData = pFrames->value.StructuredData().data;
					}
					if (frameData)
					{
						auto numFrames = frameData->Length() / sizeof(DKTransformUnit);
						const DKTransformUnit* transforms = reinterpret_cast<const DKTransformUnit*>(frameData->LockShared());
						sn.frames.Add(transforms, numFrames);
						frameData->UnlockShared();
						sn.name = pName->value.String();
						return true;
					}
				}
			}
			return false;
		}
		void GetKeyframeNodeVariant(DKVariant& v, const KeyframeNode& kn) const
		{
			v.SetValueType(DKVariant::TypePairs);
			v.Pairs().Insert(L"name", (const DKVariant::VString&)kn.name);

			DKVariant scaleKeys(DKVariant::TypeStructData);
			DKVariant rotationKeys(DKVariant::TypeStructData);
			DKVariant translationKeys(DKVariant::TypeStructData);

			DKVariant::VStructuredData& scaleData = scaleKeys.StructuredData();
			DKVariant::VStructuredData& rotationData = rotationKeys.StructuredData();
			DKVariant::VStructuredData& translationData = translationKeys.StructuredData();

			scaleData.data = DKOBJECT_NEW DKBuffer((const KeyframeNode::ScaleKey*)kn.scaleKeys, kn.scaleKeys.Count() * sizeof(KeyframeNode::ScaleKey));
			scaleData.elementSize = sizeof(KeyframeNode::ScaleKey);
			scaleData.layout.Add(DKVariant::StructElem::Arithmetic4, sizeof(KeyframeNode::ScaleKey) / 4);

			rotationData.data = DKOBJECT_NEW DKBuffer((const KeyframeNode::RotationKey*)kn.rotationKeys, kn.rotationKeys.Count() * sizeof(KeyframeNode::RotationKey));
			rotationData.elementSize = sizeof(KeyframeNode::RotationKey);
			rotationData.layout.Add(DKVariant::StructElem::Arithmetic4, sizeof(KeyframeNode::RotationKey) / 4);

			translationData.data = DKOBJECT_NEW DKBuffer((const KeyframeNode::TranslationKey*)kn.translationKeys, kn.translationKeys.Count() * sizeof(KeyframeNode::TranslationKey));
			translationData.elementSize = sizeof(KeyframeNode::TranslationKey);
			translationData.layout.Add(DKVariant::StructElem::Arithmetic4, sizeof(KeyframeNode::TranslationKey) / 4);

			v.Pairs().Insert(L"scaleKeys", scaleKeys);
			v.Pairs().Insert(L"rotationKeys", rotationKeys);
			v.Pairs().Insert(L"translationKeys", translationKeys);
		}
		bool SetKeyframeNodeVariant(const DKVariant& v, KeyframeNode& kn) const
		{
			if (v.ValueType() == DKVariant::TypePairs)
			{
				const DKVariant::VPairs::Pair* pName = v.Pairs().Find(L"name");
				const DKVariant::VPairs::Pair* pScaleKeys = v.Pairs().Find(L"scaleKeys");
				const DKVariant::VPairs::Pair* pRotationKeys = v.Pairs().Find(L"rotationKeys");
				const DKVariant::VPairs::Pair* pTranslationKeys = v.Pairs().Find(L"translationKeys");

				if (pName && pName->value.ValueType() == DKVariant::TypeString &&
					pScaleKeys && pRotationKeys && pTranslationKeys)
				{
					const DKData* scaleData = NULL;
					const DKData* rotationData = NULL;
					const DKData* translationData = NULL;

					if (pScaleKeys->value.ValueType() == DKVariant::TypeData)
						scaleData = &(pScaleKeys->value.Data());
					else if (pScaleKeys->value.ValueType() == DKVariant::TypeStructData)
						scaleData = pScaleKeys->value.StructuredData().data;

					if (pRotationKeys->value.ValueType() == DKVariant::TypeData)
						rotationData = &(pRotationKeys->value.Data());
					else if (pRotationKeys->value.ValueType() == DKVariant::TypeStructData)
						rotationData = pRotationKeys->value.StructuredData().data;

					if (pTranslationKeys->value.ValueType() == DKVariant::TypeData)
						translationData = &(pTranslationKeys->value.Data());
					else if (pTranslationKeys->value.ValueType() == DKVariant::TypeStructData)
						translationData = pTranslationKeys->value.StructuredData().data;

					if (scaleData && rotationData && translationData)
					{
						kn.scaleKeys.Add((const KeyframeNode::ScaleKey*)scaleData->LockShared(), scaleData->Length() / sizeof(KeyframeNode::ScaleKey));
						kn.rotationKeys.Add((const KeyframeNode::RotationKey*)rotationData->LockShared(), rotationData->Length() / sizeof(KeyframeNode::RotationKey));
						kn.translationKeys.Add((const KeyframeNode::TranslationKey*)translationData->LockShared(), translationData->Length() / sizeof(KeyframeNode::TranslationKey));

						scaleData->UnlockShared();
						rotationData->UnlockShared();
						translationData->UnlockShared();

						kn.name = pName->value.String();
						return true;
					}
				}
			}
			return false;
		}
		void GetSamplingNode(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeArray);
			v.Array().Reserve(target->nodes.Count());
			for (const Node* n : target->nodes)
			{
				if (n->type == Node::NodeTypeSampling)
				{
					DKVariant::VArray::Index idx = v.Array().Add(DKVariant(DKVariant::TypeUndefined));
					GetSamplingNodeVariant(v.Array().Value(idx), *static_cast<const SamplingNode*>(n));
				}
			}
		}
		void SetSamplingNode(DKVariant& v)
		{
			for (size_t i = 0; i < v.Array().Count(); ++i)
			{
				SamplingNode sn;
				if (SetSamplingNodeVariant(v.Array().Value(i), sn))
				{
					target->AddNode(&sn);
				}
			}
		}
		bool CheckSamplingNode(const DKVariant& v)
		{
			return v.ValueType() == DKVariant::TypeArray;
		}
		void GetKeyframeNode(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeArray);
			v.Array().Reserve(target->nodes.Count());
			for (const Node* n : target->nodes)
			{
				if (n->type == Node::NodeTypeKeyframe)
				{
					DKVariant::VArray::Index idx = v.Array().Add(DKVariant(DKVariant::TypeUndefined));
					GetKeyframeNodeVariant(v.Array().Value(idx), *static_cast<const KeyframeNode*>(n));
				}
			}
		}
		void SetKeyframeNode(DKVariant& v)
		{
			for (size_t i = 0; i < v.Array().Count(); ++i)
			{
				KeyframeNode kn;
				if (SetKeyframeNodeVariant(v.Array().Value(i), kn))
				{
					target->AddNode(&kn);
				}
			}
		}
		bool CheckKeyframeNode(const DKVariant& v)
		{
			return v.ValueType() == DKVariant::TypeArray;
		}
		void GetDuration(DKVariant& v) const
		{
			v = (DKVariant::VFloat)target->Duration();
		}
		void SetDuration(DKVariant& v)
		{
			target->SetDuration(v.Float());
		}
		bool CheckDuration(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeFloat;
		}
		void Callback(State s)
		{
			if (s == StateDeserializeBegin)
			{
				target->RemoveAllNodes();
			}
		}
		DKObject<DKAnimation> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
