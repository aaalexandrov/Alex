#pragma once

#include "output_pass.h"
#include "device.h"
#include "rttr_factory.h"

NAMESPACE_BEGIN(gr1)

class Image;

class RenderCommand {
	RTTR_ENABLE()
public:
	virtual ~RenderCommand() {}
};

class RenderPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	RenderPass(Device &device) : OutputPass(device), _cmdFactory(RttrDiscriminator::Tag, device.GetResourceDiscriminatorType()) {}

	virtual std::shared_ptr<RenderCommand> CreateCommand(rttr::type cmdType) { return _cmdFactory.CreateInstanceShared<RenderCommand>(cmdType); }
	
	template<typename CmdType>
	std::shared_ptr<CmdType> CreateCommand() { return std::static_pointer_cast<CmdType>(CreateCommand(rttr::type::get<>(CmdType))); }

	virtual void AddAttachment(std::shared_ptr<Image> img);

	virtual void AddCommand(std::shared_ptr<RenderCommand> cmd);

protected:
	RttrFactory _cmdFactory;
	std::vector<std::shared_ptr<RenderCommand>> _commands;
};

NAMESPACE_END(gr1)

