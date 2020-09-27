#pragma once
#include <chrono>


namespace imgui {

    using time_point = std::chrono::system_clock::time_point;
    /**
     * Creates a date picker - level needs to exist per instance.
     * A date is selected if the function returns true and level == 0
     */
    bool date_picker(char const* id, int& level, time_point& to_edit, time_point const start, time_point const end);
    /**
     * Trivial three combo box time selection - returns true on change
     */
    bool day_time_picker(char const* id, time_point& to_edit);
}
