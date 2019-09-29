#pragma once

#include "../model.h"

NAMESPACE_BEGIN(gr)

class ModelVk : public Model {
public:

  util::TypeInfo *GetType() override;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::ModelVk, gr::Model)