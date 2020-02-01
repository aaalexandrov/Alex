#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

enum class QueueRole;
struct QueueVk;

class ImageTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:
	ImageTransitionPassVk(Device &device);

	void Prepare(PassData *passData) override;
	void Execute(PassData *passData) override;

public:
	bool GetTransitionQueueInfo(QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue);

	vk::UniqueCommandBuffer _srcCmds, _dstCmds;
	vk::UniqueSemaphore _queueTransitionSemaphore;
};

NAMESPACE_END(gr1)