#include "JM.h"
#include "PathNode.h"
#include "PathEdge.h"

namespace jm
{

	PathNode::PathNode(const maths::Vector3 &position)
		: PhysicsObject3D()
	{
		SetPosition(position);
	}

	PathNode::~PathNode()
	{
	}

	bool PathNode::IsOnList(const std::vector<PathNode *> &list)
	{
		return std::find(list.begin(), list.end(), this) != list.end();
	}

	float PathNode::HeuristicValue(const PathNode &other) const
	{
		return (GetWorldSpaceTransform().GetPositionVector() - other.GetWorldSpaceTransform().GetPositionVector()).Length();
	}

}