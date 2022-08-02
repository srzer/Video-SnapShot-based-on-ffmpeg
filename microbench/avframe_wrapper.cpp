#include <exception>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

#include "avframe_wrapper.hpp"

extern "C" int get_sample_frame(const char *filename, int frame_num, AVFrame *frame);

namespace microbench
{

avframe_wrapper::avframe_wrapper() : m_frame(av_frame_alloc())
{
	if (!m_frame) throw std::bad_alloc();
}
avframe_wrapper::~avframe_wrapper()
{
	if (m_frame) {
		av_frame_unref(m_frame);
		av_frame_free(&m_frame);
	}
}
PixFmt avframe_wrapper::pixelFormat() const
{
	switch (m_frame->format) {
	case AV_PIX_FMT_RGBA:
		return PixFmt::eRGBA;
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
		return PixFmt::eYUV420P;
	case AV_PIX_FMT_YUV444P:
	case AV_PIX_FMT_YUVJ444P:
		return PixFmt::eYUV444P;
	default:
		return PixFmt::eUNKNOWN;
	}
}
avframe_wrapper::avframe_wrapper(const avframe_wrapper &that) : m_frame(av_frame_alloc())
{
	if (!m_frame) throw std::bad_alloc();
	av_frame_ref(this->m_frame, that.m_frame);
}

avframe_wrapper::avframe_wrapper(avframe_wrapper &&that) noexcept
{
	m_frame = that.m_frame;
	that.m_frame = nullptr;
}

avframe_wrapper &avframe_wrapper::operator=(avframe_wrapper rhs)
{
	swap(rhs, *this);
	return *this;
}

void swap(avframe_wrapper &lhs, avframe_wrapper &rhs) noexcept
{
	using std::swap;
	swap(lhs.m_frame, rhs.m_frame);
}
int avframe_wrapper::width() const
{
	return m_frame->width;
}
int avframe_wrapper::height() const
{
	return m_frame->height;
}
int *avframe_wrapper::linesize() const
{
	return m_frame->linesize;
}
uint8_t **avframe_wrapper::data() const
{
	return m_frame->data;
}
avframe_wrapper getSampleFrame()
{
	// Does different input changes performance?
	auto picture = std::filesystem::path {TEA_PROJECT_SAMPLE_PATH} / "small_sample.png";
//	auto picture = std::filesystem::path {TEA_PROJECT_SAMPLE_PATH} / "yuv420.mkv";

	int ret;
	char error_buf[AV_ERROR_MAX_STRING_SIZE];
	avframe_wrapper frame{};
	const char *path;
#ifdef _WIN32
	const wchar_t *wpath = picture.c_str();
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, 0, 0);
	std::vector<char> cpath(utf8_size + 1);
	WideCharToMultiByte(CP_UTF8, 0, wpath, -1, cpath.data(), utf8_size, 0, 0);
	path = cpath.data();
#else
	path = picture.c_str();
#endif

	if ((ret = get_sample_frame(path, 1, frame.get_underlying_ptr())) < 0) {
		throw std::runtime_error(av_make_error_string(error_buf, AV_ERROR_MAX_STRING_SIZE, ret));
	}

	return frame;
}

} // namespace microbench
