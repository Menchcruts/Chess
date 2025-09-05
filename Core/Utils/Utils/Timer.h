#include <chrono>
#include <format>
#include <iostream>
#include <string_view>
#include <utility>

template<class D>
concept ChronoDuration =
    requires(std::chrono::steady_clock::duration d)
{
    typename D::rep;
    typename D::period;
    std::chrono::duration_cast<D>(d);
};

template<ChronoDuration Dur = std::chrono::microseconds>
class Timer
{
public:
    using clock = std::chrono::steady_clock;

    explicit Timer(std::string_view label = {}, std::ostream& out = std::cout)
        : label_(label), out_(&out), start_(clock::now())
    {
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    Timer(Timer&& other) = delete;
    Timer& operator=(Timer&&) = delete;

    ~Timer() noexcept
    {
        // Destructors must not throw.
        try
        {
            auto d = elapsed<Dur>();
            if (label_.empty())
            {
                *out_ << std::format("{}\n", d);
            }
            else
            {
                *out_ << std::format("{}: {}\n", label_, d);
            }
        }
        catch (...)
        {
            // Fallback if formatting throws (should be rare).
            *out_ << label_ << ": "
                << std::chrono::duration_cast<std::chrono::microseconds>(
                    clock::now() - start_).count()
                << "us\n";
        }
    }

    // Access the elapsed duration without stopping/printing.
    template<ChronoDuration D2 = Dur>
    [[nodiscard]] D2 elapsed() const
    {
        return std::chrono::duration_cast<D2>(clock::now() - start_);
    }

    // Reset the start point (continues timing).
    void reset() { start_ = clock::now(); }

private:
    std::string_view label_;
    std::ostream* out_;
    clock::time_point start_;
};