#include "model_instance.h"

NAMESPACE_BEGIN(gr)

void ModelInstance::SetModel(Model *model)
{
  _model = model->SharedFromType<Model>();
  Invalidate();
}


NAMESPACE_END(gr)

