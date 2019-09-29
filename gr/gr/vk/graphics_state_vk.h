#pragma once

#include "../graphics_state.h"

NAMESPACE_BEGIN(gr)

class GraphicsStateVk : public GraphicsState {
public:

  void Invalidate() override;
};

NAMESPACE_END(gr)