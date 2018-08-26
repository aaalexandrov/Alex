#include "stdafx.h"

#include "Octree.h"
#include "util.h"

Octree::Octree(AABB const & bound, float minSize)
{
  m_Root = std::make_unique<Node>();
  m_Bound = bound;
  m_MinSize = minSize;
}

void Octree::Add(Bounded const & item)
{
  GrowUpToInclude(item.GetBound());
  Add(m_Root.get(), m_Bound, item);
}

void Octree::Remove(Bounded const & item)
{
  Remove(m_Root.get(), m_Bound, item);
}

void Octree::Intersect(AABB const & bound, IntersectAABBCallback callback)
{
  Intersect(m_Root.get(), m_Bound, bound, callback);
}

void Octree::Intersect(Line3f const & line, IntersectLineCallback callback)
{
  Intersect(m_Root.get(), m_Bound, line, callback);
}

void Octree::GrowUpToInclude(AABB const &bound)
{
  while (!m_Bound.contains(bound)) {
    Vector3f size = m_Bound.sizes();
    int childInd = 0;
    for (int dim = 0; dim < 3; ++dim) {
      if (bound.min()(dim) < m_Bound.min()(dim)) {
        m_Bound.min()(dim) -= size(dim);
        childInd |= 1 << dim;
      } else {
        m_Bound.max()(dim) += size(dim);
      }
      std::unique_ptr<Node> prevRoot(std::move(m_Root));
      m_Root = std::make_unique<Node>();
      m_Root->m_Children[childInd] = std::move(prevRoot);
    }
  }
}

void Octree::Add(Node *node, AABB const & bound, Bounded const & item)
{
  AABB nodeBound = bound;
  while (true) {
    int childInd = GetChildIndex(nodeBound, item.GetBound());
    if (childInd < 0)
      break;
    Node *child = node->m_Children[childInd].get();
    if (!child && nodeBound.sizes().minCoeff() > m_MinSize) {
      node->m_Children[childInd] = std::make_unique<Node>();
      child = node->m_Children[childInd].get();
    }
    if (!child)
      break;
    ComputeChildBound(nodeBound, childInd, nodeBound);
    node = child;
  }
  node->m_Items.push_back(&item);
}

void Octree::Remove(Node * node, AABB const & bound, Bounded const & item)
{
  AABB nodeBound = bound;
  while (true) {
    int childInd = GetChildIndex(nodeBound, item.GetBound());
    if (childInd < 0)
      break;
    Node *child = node->m_Children[childInd].get();
    if (!child)
      break;
    ComputeChildBound(nodeBound, childInd, nodeBound);
    node = child;
  }
  RemoveFromVector(node->m_Items, &item);
}

bool Octree::Intersect(Node * node, AABB const & nodeBound, AABB const & intersectBound, IntersectAABBCallback callback)
{
  if (!nodeBound.intersects(intersectBound))
    return true;
  for (auto item : node->m_Items) {
    if (!item->GetBound().intersects(intersectBound))
      continue;
    if (!callback(item))
      return false;
  }
  for (int i = 0; i < 8; ++i) {
    Node *child = node->m_Children[i].get();
    if (!child)
      continue;
    AABB childBound;
    ComputeChildBound(nodeBound, i, childBound);
    if (!Intersect(child, childBound, intersectBound, callback))
      return false;
  }
  return true;
}

bool Octree::Intersect(Node * node, AABB const & nodeBound, Line3f const & intersectLine, IntersectLineCallback callback)
{
  float t0, t1;
  if (!LineBoxIntersect(intersectLine, nodeBound, t0, t1))
    return true;
  for (auto item : node->m_Items) {
    if (!LineBoxIntersect(intersectLine, item->GetBound(), t0, t1))
      continue;
    if (!callback(item, t0, t1))
      return false;
  }
  for (int i = 0; i < 8; ++i) {
    Node *child = node->m_Children[i].get();
    if (!child)
      continue;
    AABB childBound;
    ComputeChildBound(nodeBound, i, childBound);
    if (!Intersect(child, childBound, intersectLine, callback))
      return false;
  }
  return true;
}

int Octree::GetChildIndex(AABB const &nodeBound, AABB const &itemBound)
{
  int ind = 0;
  Vector3f mid = nodeBound.center();
  for (int dim = 0; dim < 3; ++dim) {
    if (itemBound.max()(dim) <= mid(dim)) {
      // index bit is already set to 0
    } else if (mid(dim) <= itemBound.min()(dim)) {
      ind |= 1 << dim;
    } else {
      return -1;
    }
  }
  return ind;
}

void Octree::ComputeChildBound(AABB const & nodeBound, int childIndex, AABB & childBound)
{
  Vector3f mid = nodeBound.center();
  childBound = nodeBound;
  for (int dim = 0; dim < 3; ++dim) {
    if (childIndex & (1 << dim))
      childBound.min()(dim) = mid(dim);
    else
      childBound.max()(dim) = mid(dim);
  }
}
