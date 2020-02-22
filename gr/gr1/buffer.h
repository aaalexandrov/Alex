#pragma once

#include "resource.h"
#include "util/enumutl.h"
#include "util/layout.h"

NAMESPACE_BEGIN(gr1)

class Buffer : public Resource {
	RTTR_ENABLE(Resource)
public:
  enum class Usage {
    None = 0,
    Vertex = 1,
    Index = 2,
    Uniform = 4,
    Staging = 8,
  };

  Buffer(Device &device) : Resource(device) {}

	virtual void Init(Usage usage, std::shared_ptr<util::LayoutElement> const &layout);

	template <typename Elem>
	void Init(Usage usage, std::vector<Elem> const &content)
	{
		Init(usage, util::CreateLayoutArray(content));
		void *mapped = Map();
		memcpy(mapped, content.data(), content.size());
		Unmap();
	}

	Usage GetUsage() const { return _usage; }
	uint32_t GetSize() const { return static_cast<uint32_t>(_layout->GetSize()); }

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

	inline std::shared_ptr<util::LayoutElement> const &GetBufferLayout() const { return _layout; }
protected:
	Usage _usage = Usage::None;
	std::shared_ptr<util::LayoutElement> _layout;
};

DEFINE_ENUM_BIT_OPERATORS(Buffer::Usage);

NAMESPACE_END(gr1)

