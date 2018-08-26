#pragma once

#include <vector>

#include "Geom.h"

class Bounded {
public:
  virtual AABB const &GetBound() const = 0;
};

class Octree {
public:
  Octree(AABB const &bound, float minSize);

  void Add(Bounded const &item);
  void Remove(Bounded const &item);

  typedef std::function<bool(Bounded const *)> IntersectAABBCallback;
  typedef std::function<bool(Bounded const *, float t0, float t1)> IntersectLineCallback;

  void Intersect(AABB const &bound, IntersectAABBCallback callback);
  void Intersect(Line3f const &line, IntersectLineCallback callback);

public:
  struct Node {
    std::vector<Bounded const *> m_Items;
    std::unique_ptr<Node> m_Children[8];
  };

  AABB m_Bound;
  std::unique_ptr<Node> m_Root;
  float m_MinSize;

  void GrowUpToInclude(AABB const &bound);

  void Add(Node *node, AABB const &bound, Bounded const &item);
  void Remove(Node *node, AABB const &bound, Bounded const &item);

  bool Intersect(Node *node, AABB const &nodeBound, AABB const &intersectBound, IntersectAABBCallback callback);
  bool Intersect(Node *node, AABB const &nodeBound, Line3f const &intersectLine, IntersectLineCallback callback);

  static int GetChildIndex(AABB const &nodeBound, AABB const &itemBound);
  static void ComputeChildBound(AABB const &nodeBound, int childIndex, AABB &childBound);
};