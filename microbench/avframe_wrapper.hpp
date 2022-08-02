//
// Created by ocean on 5/29/22.
//

#ifndef AVFRAME_WRAPPER_HPP
#define AVFRAME_WRAPPER_HPP

#include <cstdint>

extern "C" struct AVFrame;

namespace microbench
{

enum class PixFmt { eRGBA, eYUV420P, eYUV444P, eUNKNOWN };

class avframe_wrapper
{
public:
	avframe_wrapper();
	virtual ~avframe_wrapper();

	avframe_wrapper(const avframe_wrapper &);
	avframe_wrapper(avframe_wrapper &&) noexcept;
	avframe_wrapper &operator=(avframe_wrapper rhs);

	friend void swap(avframe_wrapper &lhs, avframe_wrapper &rhs) noexcept;

	[[nodiscard]] AVFrame *get_underlying_ptr() const { return m_frame; }
	[[nodiscard]] PixFmt pixelFormat() const;
	[[nodiscard]] int width() const;
	[[nodiscard]] int height() const;
	[[nodiscard]] int *linesize() const;
	[[nodiscard]] uint8_t **data() const;

private:
	AVFrame *m_frame;
};

[[nodiscard]] avframe_wrapper getSampleFrame();

} // namespace microbench

#endif //AVFRAME_WRAPPER_HPP
