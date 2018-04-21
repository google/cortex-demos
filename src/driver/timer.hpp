#pragma once

#include <cstdint>

namespace driver {

class Timer {
    public:
        enum class ID {
            RTC0,
            RTC1,
            RTC2,
            RTC3,
            RTC4,
            RTC5,
            RTC6,
        };

        Timer(uint32_t base);
        virtual ~Timer() {}

        static Timer* get_by_id(ID id);

        void start();
        void stop();

    private:
        const uint32_t base_;
        int irq_n_;

};

}  // namespace driver
