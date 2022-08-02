#include <memory>

#include <benchmark/benchmark.h>

#include "PixelConverter.hpp"
#include "avframe_wrapper.hpp"

using namespace microbench;

class FrameViewBase
{
protected:
	const avframe_wrapper &m_frame;
	int width, height;

	explicit FrameViewBase(const avframe_wrapper &frame) : m_frame(frame)
	{
		width = frame.width();
		height = frame.height();
	};

public:
	virtual float averageLuminance()
	{
		float accmu = 0.0f;
		for (int i = 0; i < width; i++)
			for (int j = 0; j < height; j++)
				accmu += getLuminance(i, j);
		return accmu / static_cast<float>(width * height);
	}
	virtual float getLuminance(int x, int y) = 0;

	static std::unique_ptr<FrameViewBase> frameViewFactory(const avframe_wrapper &frame);

	virtual ~FrameViewBase() = default;
};

class RGBAFrameView : public FrameViewBase
{
public:
	explicit RGBAFrameView(const avframe_wrapper &frame)
		: FrameViewBase(frame)
		, stride(frame.linesize()[0])
		, data(frame.data()[0])
	{
	}

	float getLuminance(int x, int y) override
	{
		auto &lookup = util::SrgbLuminance::getSingleton();

		uint8_t *ptr = data + (y * stride + x);
		uint8_t r = ptr[0], g = ptr[1], b = ptr[2];
		return lookup.getLinearLuminance(r, g, b);
	}

private:
	int stride;
	uint8_t *data;
};

class YUVFrameView : public FrameViewBase
{
public:
	explicit YUVFrameView(const avframe_wrapper &frame) : FrameViewBase(frame)
	{
		for (int i = 0; i < 3; i++) {
			stride[i] = frame.linesize()[i];
			data[i] = frame.data()[i];
		}
	}

	float getLuminance(int x, int y) override { return m_frame.data()[0][y * stride[0] + x]; }

private:
	int stride[3]{};
	uint8_t *data[3]{};
};

std::unique_ptr<FrameViewBase> FrameViewBase::frameViewFactory(const avframe_wrapper &frame)
{
	switch (frame.pixelFormat()) {
	case PixFmt::eRGBA:
		return std::make_unique<RGBAFrameView>(frame);
	case PixFmt::eYUV420P:
		return std::make_unique<YUVFrameView>(frame);
	default:
		std::terminate();
	}
}

static void InheritanceAvgLum(benchmark::State &state)
{
	auto frame = microbench::getSampleFrame();
	auto view = FrameViewBase::frameViewFactory(frame);

	float acc = 0.0f;
	for ([[maybe_unused]] auto _ : state) {
		benchmark::DoNotOptimize(acc += view->averageLuminance());
	}
}

BENCHMARK(InheritanceAvgLum); // NOLINT
