//
// Created by ocean on 5/29/22.
//

#ifndef PIXELCONVERTER_HPP
#define PIXELCONVERTER_HPP

#include <cstdint>

namespace util
{

class SrgbLuminance
{
public:
	static const SrgbLuminance &getSingleton();

	SrgbLuminance(const SrgbLuminance &) = delete;
	SrgbLuminance(SrgbLuminance &&) = delete;

	[[nodiscard]] float getLinearLuminance(uint8_t r, uint8_t g, uint8_t b) const;
	[[nodiscard]] float getLinearLuminance(int r, int g, int b) const;

private:
	SrgbLuminance();

	float m_YLookupR[256];
	float m_YLookupG[256];
	float m_YLookupB[256];
};

} // namespace util

#endif //PIXELCONVERTER_HPP
