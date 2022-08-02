#include <benchmark/benchmark.h>
#include <exception>
#include <type_traits>

#include "PixelConverter.hpp"
#include "avframe_wrapper.hpp"

using namespace microbench;

struct BaseView {
	int width, height;

	explicit BaseView(const avframe_wrapper &frame)
	{
		width = frame.width();
		height = frame.height();
	}
};

template<typename T> float averageLuminance(const T &view)
{
	float accmu = 0.0f;
	for (int i = 0; i < view.width; i++)
		for (int j = 0; j < view.height; j++)
			accmu += getLuminance(view, i, j);
	return accmu / static_cast<float>(view.width * view.height);
}

template<typename T> float getLuminance(const T &view, int x, int y) = delete;

struct RGBAView : public BaseView {
	int stride;
	uint8_t *data;

	explicit RGBAView(const avframe_wrapper &frame)
		: BaseView(frame)
		, stride(frame.linesize()[0])
		, data(frame.data()[0])
	{
	}
};

template<> float getLuminance(const RGBAView &view, int x, int y)
{
	auto &lookup = util::SrgbLuminance::getSingleton();

	uint8_t *ptr = view.data + (y * view.stride + x);
	uint8_t r = ptr[0], g = ptr[1], b = ptr[2];
	return lookup.getLinearLuminance(r, g, b);
}

struct YUVView : public BaseView {
	int stride[3]{};
	uint8_t *data[3]{};

	explicit YUVView(const avframe_wrapper &frame) : BaseView(frame)
	{
		for (int i = 0; i < 3; i++) {
			stride[i] = frame.linesize()[i];
			data[i] = frame.data()[i];
		}
	}
};

template<> float getLuminance(const YUVView &view, int x, int y)
{
	return view.data[0][y * view.stride[0] + x];
}

// Type-erased factory class
class OverView
{
public:
	float averageLuminance() { return m_averageLuminance(m_view); }

	static OverView OverViewFactory(const avframe_wrapper &frame)
	{
		switch (frame.pixelFormat()) {
		case PixFmt::eRGBA:
			return {RGBAView{frame}};
		case PixFmt::eYUV420P:
			return {YUVView{frame}};
		default:
			std::terminate();
		}
	}

	OverView(const OverView &) = delete;
	OverView(OverView &&) = delete;

	~OverView() { m_deleter(m_view); };

private:
	// This is why you need C++20 concept
	template<typename T, std::enable_if_t<std::is_base_of_v<BaseView, T>, bool> = true>
	OverView(const T &view) // NOLINT
	{
		m_view = new T(view);
		m_averageLuminance = [](void *v) { return ::averageLuminance<T>(*(T *)v); };
		m_deleter = [](void *v) { delete (T *)v; };
	}

	void *m_view;
	float (*m_averageLuminance)(void *);
	void (*m_deleter)(void *);
};

static void TemplateAvgLum(benchmark::State &state)
{
	auto frame = microbench::getSampleFrame();
	auto view = OverView::OverViewFactory(frame);

	float acc = 0.0f;
	for ([[maybe_unused]] auto _ : state) {
		benchmark::DoNotOptimize(acc += view.averageLuminance());
	}
}

BENCHMARK(TemplateAvgLum); // NOLINT
