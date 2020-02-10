#include "sampler.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr1)

size_t Sampler::SamplerData::GetHash() const
{
	size_t hash = util::GetHash(_magFilter);
	hash = util::GetHash(_minFilter, hash);
	hash = util::GetHash(_mipMapMode, hash);
	hash = util::GetHash(_addressModes, hash);
	hash = util::GetHash(_mipLodBias, hash);
	hash = util::GetHash(_anisotropic, hash);
	hash = util::GetHash(_maxAnisotropy, hash);
	hash = util::GetHash(_compareFunc, hash);
	hash = util::GetHash(_minLod, hash);
	hash = util::GetHash(_maxLod, hash);
	hash = util::GetHash(_borderColor, hash);
	hash = util::GetHash(_normalizedCoordinates, hash);
	return size_t();
}

bool Sampler::SamplerData::operator==(SamplerData const &other) const
{
	return _magFilter == other._magFilter
		&& _minFilter == other._minFilter
		&& _mipMapMode == other._mipMapMode
		&& _addressModes == other._addressModes
		&& _mipLodBias == other._mipLodBias
		&& _anisotropic == other._anisotropic
		&& _maxAnisotropy == other._maxAnisotropy
		&& _compareFunc == other._compareFunc
		&& _minLod == other._minLod
		&& _maxLod == other._maxLod
		&& _borderColor == other._borderColor
		&& _normalizedCoordinates == other._normalizedCoordinates;
}

void Sampler::Init()
{
	_state = ResourceState::ShaderRead;
}

NAMESPACE_END(gr1)

