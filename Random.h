#pragma once

#include <random>

namespace tora {
    class Random {
    public:
        Random()
            : mt(rd()), distFloat(0.0f, 1.0f), distDouble(0.0, 1.0) {}

        float getRandomFloat() {
            return distFloat(mt);
        }

        double getRandomDouble() {
            return distDouble(mt);
        }

        template<typename T>
        T getRandomBetween(T min, T max) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(mt);
        }

    private:
        std::random_device rd;
        std::mt19937 mt;
        std::uniform_real_distribution<float> distFloat;
        std::uniform_real_distribution<double> distDouble;
    };
} // namespace tora