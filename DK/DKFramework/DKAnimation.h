//
//  File: DKAnimation.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKTransform.h"
#include "DKAnimationController.h"

////////////////////////////////////////////////////////////////////////////////
// DKAnimation
// Animation object. It has frame data as DKTransformUnit objects.
// object can contains multiple nodes, which can be used for skinning animation.
// Each node has it's own animation data as DKTransformUnit objects.
//
// You need to create controller object (DKAnimationController) to animate
// any kind of DKModel. (don't have to create controller for calculation only)
//
// Note:
//   This class can handles affine-transform only.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKAnimation : public DKResource
	{
	public:
		typedef long NodeIndex;
		static const NodeIndex invalidNodeIndex = -1;

		struct Node
		{
			enum NodeType
			{
				NodeTypeSampling,
				NodeTypeKeyframe,
			};
			DKFoundation::DKString	name;
			const NodeType	type;

			Node(NodeType t) : type(t) {}
			virtual ~Node(void) {}
			virtual bool IsEmpty(void) const = 0;
		};
		struct SamplingNode : public Node
		{
			DKFoundation::DKArray<DKTransformUnit>	frames;
			SamplingNode(void) : Node(NodeTypeSampling) {}
			bool IsEmpty(void) const			{return frames.IsEmpty();}
		};
		struct KeyframeNode : public Node
		{
			template <typename KEY_TYPE> struct Key
			{
				float		time;
				KEY_TYPE	key;
			};
			typedef Key<DKVector3>		ScaleKey;
			typedef Key<DKQuaternion>	RotationKey;
			typedef Key<DKVector3>		TranslationKey;

			DKFoundation::DKArray<ScaleKey>			scaleKeys;
			DKFoundation::DKArray<RotationKey>		rotationKeys;
			DKFoundation::DKArray<TranslationKey>	translationKeys;

			KeyframeNode(void) : Node(NodeTypeKeyframe) {}
			bool IsEmpty(void) const		{return translationKeys.IsEmpty() && rotationKeys.IsEmpty() && scaleKeys.IsEmpty();}
		};
		struct NodeSnapshot
		{
			DKFoundation::DKString	name;
			DKTransformUnit			transform;
		};

		DKAnimation(void);
		~DKAnimation(void);

		// Node insertion
		bool		AddNode(const Node* node);
		bool		AddSamplingNode(const DKFoundation::DKString& name, const DKTransformUnit* frames, size_t numFrames);
		bool		AddKeyframeNode(const DKFoundation::DKString& name,
									const KeyframeNode::ScaleKey* scaleKeys, size_t numSk,
									const KeyframeNode::RotationKey* rotationKeys, size_t numRk,
									const KeyframeNode::TranslationKey* translationKeys, size_t numTk);

		void		RemoveNode(const DKFoundation::DKString& name);
		void		RemoveAllNodes(void);
		size_t		NodeCount(void) const;
		NodeIndex	IndexOfNode(const DKFoundation::DKString& name) const;
		const Node*	NodeAtIndex(NodeIndex index) const;

		// calculate transform at time ( 0.0 <= t <= 1.0 )
		bool GetNodeTransform(NodeIndex index, float t, DKTransformUnit& output) const;
		bool GetNodeTransform(const DKFoundation::DKString& name, float t, DKTransformUnit& output) const;

		// generate snap-shot.
		// snap-shot can be combined with other animation object. (interpolated altogether)
		DKFoundation::DKArray<NodeSnapshot> CreateSnapshot(float t) const;

		// create object from snap-shots (useful to interpolate transition of two animations)
		static DKFoundation::DKObject<DKAnimation> Create(DKFoundation::DKArray<SamplingNode>* samples, DKFoundation::DKArray<KeyframeNode>* keyframes, float duration);
		static DKFoundation::DKObject<DKAnimation> Create(DKFoundation::DKArray<NodeSnapshot>* begin, DKFoundation::DKArray<NodeSnapshot>* end, float duration);

		// get affine-transform of specified node at time.
		static DKTransformUnit GetTransform(const Node& node, float time);

		// convert node (keyframe to sampling, or vice versa.)
		// for converting to key-frame, timing tick is 1.0/(frames-1) unit. (first frame:0, last frame:frames-1)
		static bool ResampleNode(const Node& source, unsigned int frames, KeyframeNode& output, float threshold = 0.000001f);
		static bool ResampleNode(const Node& source, unsigned int frames, SamplingNode& output);

		// set animation duration.
		void	SetDuration(float d);
		float	Duration(void) const;

		// create loop controller. (useful to apply repeated animation to DKModel)
		DKFoundation::DKObject<DKAnimationController> CreateLoopController(void);

		// serializer object.
		DKFoundation::DKObject<DKSerializer> Serializer(void);
	private:
		float	duration;

		DKFoundation::DKMap<DKFoundation::DKString, size_t> nodeIndexMap; // for fast search
		DKFoundation::DKArray<Node*>	nodes;
	};
}
