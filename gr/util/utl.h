#pragma once

#include "namespace.h"

NAMESPACE_BEGIN(util)

template <typename Container>
inline typename Container::mapped_type FindOrDefault(Container const &container, typename Container::key_type key, typename Container::mapped_type defaultValue)
{
  auto it = container.find(key);
  if (it == container.end())
    return defaultValue;
  return it->second;
}

template <typename Obj, typename PointedObj = Obj>
std::shared_ptr<PointedObj> SharedFromThis(Obj *obj)
{
  if (!obj)
    return std::shared_ptr<PointedObj>();
  return std::static_pointer_cast<PointedObj>(obj->shared_from_this());
}

NAMESPACE_END(util)