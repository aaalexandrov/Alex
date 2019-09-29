#pragma once

#include "../model_instance.h"

NAMESPACE_BEGIN(gr)

class ModelInstanceVk : public ModelInstance {
public:

  util::TypeInfo *GetType() override;
};


NAMESPACE_END(gr)

RTTI_BIND(gr::ModelInstanceVk, gr::ModelInstance)