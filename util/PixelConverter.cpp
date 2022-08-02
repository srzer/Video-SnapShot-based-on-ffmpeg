#include <cmath>

#include "PixelConverter.hpp"

namespace util
{

SrgbLuminance::SrgbLuminance() // NOLINT: all array elements initialized manually
{
	for (int i = 0; i < 256; i++) {
		float sfloat = (float)i / 255.0f;
		float linear = powf(sfloat, 2.2);
		m_YLookupR[i] = linear * 0.2126f;
		m_YLookupG[i] = linear * 0.7152f;
		m_YLookupB[i] = linear * 0.0722f;
	}
}
const SrgbLuminance &SrgbLuminance::getSingleton()
{
	static SrgbLuminance singleton{};
	return singleton;
}
float SrgbLuminance::getLinearLuminance(uint8_t r, uint8_t g, uint8_t b) const
{
	return m_YLookupR[r] + m_YLookupG[g] + m_YLookupB[b];
}
float SrgbLuminance::getLinearLuminance(int r, int g, int b) const
{
	if (r < 0 || r > 255) return -1;
	if (g < 0 || g > 255) return -1;
	if (b < 0 || b > 255) return -1;
	return getLinearLuminance(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
}

} // namespace util
