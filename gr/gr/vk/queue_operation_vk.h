#pragma once

#include "../queue_operation.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

struct QueueVk;

class QueueOperationVk : public QueueOperation {
public:
};

NAMESPACE_END(gr)

RTTI_BIND(gr::QueueOperationVk, gr::QueueOperation)