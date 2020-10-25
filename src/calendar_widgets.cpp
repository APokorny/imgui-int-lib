#include "imgui/widgets.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <algorithm>

// C++20 backport :(
namespace imgui
{
using days     = std::chrono::duration<int32_t, std::ratio<86400>>;
using hours    = std::chrono::hours;
using minutes  = std::chrono::minutes;
using seconds  = std::chrono::seconds;
using duration = time_point::duration;
namespace detail
{
// Returns true if year is leap year (366 days long)
inline bool is_leap_year(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }
// Returns the number of days in a month, accounting for Feb. leap years. #month is zero indexed.
inline uint8_t days_in_month(int year, int month)
{
    static const uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1] + (month == 2 && is_leap_year(year));
}

struct ymdhms
{
    int     year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    void set_ymd(int y, uint8_t m, uint8_t d)
    {
        year  = y;
        month = m;
        day   = d;
    }
    void next_month()
    {
        ++month;
        if (month == 13)
        {
            ++year;
            month = 1;
        }
        day = std::min(days_in_month(year, month), day);
    }
    void previous_month()
    {
        --month;
        if (month == 0)
        {
            --year;
            month = 12;
        }
        day = std::min(days_in_month(year, month), day);
    }
};

ymdhms from_duration(duration in)
{
    auto const dp = std::chrono::duration_cast<days>(in);
    auto const t  = in - abs(dp);

    auto const z   = dp.count() + 719468;
    auto const era = (z >= 0 ? z : z - 146096) / 146097;
    auto const doe = static_cast<unsigned>(z - era * 146097);                // [0, 146096]
    auto const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
    auto const y   = static_cast<days::rep>(yoe) + era * 400;
    auto const doy = doe - (365 * yoe + yoe / 4 - yoe / 100);  // [0, 365]
    auto const mp  = (5 * doy + 2) / 153;                      // [0, 11]
    auto const d   = doy - (153 * mp + 2) / 5 + 1;             // [1, 31]
    auto const m   = mp < 10 ? mp + 3 : mp - 9;                // [1, 12]
    using std::chrono::duration_cast;
    using std::chrono::hours;
    using std::chrono::minutes;
    using std::chrono::seconds;
    return ymdhms{y + (m <= 2),
                  static_cast<uint8_t>(m),
                  static_cast<uint8_t>(d),
                  static_cast<uint8_t>(duration_cast<hours>(t).count()),
                  static_cast<uint8_t>(duration_cast<minutes>(t - duration_cast<hours>(t)).count()),
                  static_cast<uint8_t>(
                      duration_cast<seconds>(t - duration_cast<hours>(t) - duration_cast<minutes>(t - duration_cast<hours>(t))).count())};
}

days to_days(ymdhms const& date)
{
    auto const y   = date.year - (date.month <= 2);
    auto const m   = static_cast<unsigned>(date.month);
    auto const d   = static_cast<unsigned>(date.day);
    auto const era = (y >= 0 ? y : y - 399) / 400;
    auto const yoe = static_cast<unsigned>(y - era * 400);             // [0, 399]
    auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;  // [0, 365]
    auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;            // [0, 146096]
    return days{era * 146097 + static_cast<int>(doe) - 719468};
}

days first_month_day(ymdhms const& date)
{
    auto const y   = date.year - (date.month <= 2);
    auto const m   = static_cast<unsigned>(date.month);
    auto const era = (y >= 0 ? y : y - 399) / 400;
    auto const yoe = static_cast<unsigned>(y - era * 400);     // [0, 399]
    auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5;  // [0, 365]
    auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;    // [0, 146096]
    return days{era * 146097 + static_cast<int>(doe) - 719468};
}

constexpr inline uint8_t weekday_from_days(int64_t z)
{
    auto u = static_cast<uint64_t>(z);
    return static_cast<uint8_t>(z >= -4 ? (u + 4) % 7 : u % 7);
}

constexpr inline duration to_duration(ymdhms const& date)
{
    auto const y   = date.year - (date.month <= 2);
    auto const m   = static_cast<unsigned>(date.month);
    auto const d   = static_cast<unsigned>(date.day);
    auto const era = (y >= 0 ? y : y - 399) / 400;
    auto const yoe = static_cast<unsigned>(y - era * 400);             // [0, 399]
    auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;  // [0, 365]
    auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;            // [0, 146096]
    return days{era * 146097 + static_cast<int>(doe) - 719468} + hours{date.hour} + minutes{date.minute} + seconds{date.second};
}
}  // namespace detail
}  // namespace imgui

bool imgui::date_picker(const char* id, int& level, imgui::time_point& to_edit, imgui::time_point const min_time,
                        imgui::time_point const max_time)
{
    ImGui::PushID(id);
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    auto month_name = [](int month) {
        static const char* names_mo[] = {"January", "February", "March",     "April",   "May",      "June",
                                         "July",    "August",   "September", "October", "November", "December"};
        return names_mo[month - 1];
    };
    auto abrv_month_name = [](int month) {
        static const char* abrvs_mo[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        return abrvs_mo[month - 1];
    };
    static const char* abrvs_wd[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    ImGuiStyle&    style   = ImGui::GetStyle();
    ImVec4         col_txt = style.Colors[ImGuiCol_Text];
    ImVec4         col_dis = style.Colors[ImGuiCol_TextDisabled];
    ImVec4         col_hi  = style.Colors[ImGuiCol_ButtonActive];
    const float    ht      = ImGui::GetFrameHeight();
    ImVec2         cell_size(ht * 1.25f, ht);
    char           buff[32];
    bool           changed = false;
    detail::ymdhms t1      = detail::from_duration(min_time.time_since_epoch());
    detail::ymdhms t2      = detail::from_duration(max_time.time_since_epoch());
    detail::ymdhms t       = detail::from_duration(to_edit.time_since_epoch());
    auto           apply   = [&to_edit, &t]() { to_edit = imgui::time_point{detail::to_duration(t)}; };

    const int min_yr = 1970;
    const int max_yr = 2999;

    // day widget
    if (level == 0)
    {
        const int last_year    = t.year - 1;
        const int next_year    = t.year + 1;
        const int last_mon     = t.month == 1 ? 12 : t.month - 1;
        const int next_mon     = t.month == 12 ? 1 : t.month + 1;
        const int days_this_mo = detail::days_in_month(t.year, t.month);
        const int days_last_mo = detail::days_in_month(t.month == 1 ? last_year : t.year, last_mon);
        const int first_wd     = detail::weekday_from_days(detail::first_month_day(t).count());
        // month year
        snprintf(buff, 32, "%s %d", month_name(t.month), t.year);
        if (ImGui::Button(buff)) level = 1;
        ImGui::SameLine(5 * cell_size.x);
        auto enable_month_down = !(t.year <= min_yr && t.month == 1);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && enable_month_down)
        {
            t.previous_month();
            apply();
        }
        ImGui::SameLine();
        auto enable_month_up = !(t.year >= max_yr && t.month == 12);
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && enable_month_up)
        {
            t.next_month();
            apply();
        }

        // render weekday abbreviations
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        for (int i = 0; i < 7; ++i)
        {
            ImGui::Button(abrvs_wd[i], cell_size);
            if (i != 6) { ImGui::SameLine(); }
        }
        ImGui::PopItemFlag();
        // 0 = last mo, 1 = this mo, 2 = next mo
        int mo  = first_wd > 0 ? 0 : 1;
        int day = mo == 1 ? 1 : days_last_mo - first_wd + 1;
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 0; j < 7; ++j)
            {
                if (mo == 0 && day > days_last_mo)
                {
                    mo  = 1;
                    day = 1;
                }
                else if (mo == 1 && day > days_this_mo)
                {
                    mo  = 2;
                    day = 1;
                }
                const int now_yr = (mo == 0 && t.month == 1) ? last_year : ((mo == 2 && t.month == 12) ? next_year : t.year);
                const int now_mo = mo == 0 ? last_mon : (mo == 1 ? t.month : next_mon);

                const bool off_mo   = mo == 0 || mo == 2;
                const bool t1_or_t2 = (t1.month == now_mo && t1.year == now_yr && t1.day == day) ||
                                      (t2.month == now_mo && t2.year == now_yr && t2.day == day);

                if (off_mo) ImGui::PushStyleColor(ImGuiCol_Text, col_dis);
                if (t1_or_t2)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
                    ImGui::PushStyleColor(ImGuiCol_Text, col_txt);
                }
                auto on_selected_day =
                    ((mo == 0 && t.month == last_mon) || (mo == 1 && t.month == now_mo) || (mo == 2 && t.month == next_mon)) &&
                    day == t.day;
                if (on_selected_day) ImGui::PushStyleColor(ImGuiCol_Button, col_hi);
                ImGui::PushID(i * 7 + j);
                snprintf(buff, 32, "%d", day);
                if (now_yr == min_yr - 1 || now_yr == max_yr + 1) { ImGui::Dummy(cell_size); }
                else if (ImGui::Button(buff, cell_size) && !changed)
                {
                    t.set_ymd(now_yr, now_mo, day);
                    changed = true;
                }
                ImGui::PopID();
                if (t1_or_t2) ImGui::PopStyleColor(2);
                if (off_mo) ImGui::PopStyleColor();
                if (on_selected_day) ImGui::PopStyleColor();
                if (j != 6) ImGui::SameLine();
                day++;
            }
        }
    }
    else if (level == 1)
    {
        t.day = 1;
        snprintf(buff, 32, "%d", t.year);
        if (ImGui::Button(buff)) level = 2;
        ImGui::SameLine(5 * cell_size.x);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && t.year > min_yr)
        {
            --t.year;
            apply();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && t.year < max_yr)
        {
            ++t.year;
            apply();
        }
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f / 4.0f;
        cell_size.y *= 7.0f / 3.0f;
        for (int mo = 1; mo < 13; ++mo)
        {
            const bool t1_or_t2 = (t1.month == mo && t1.year == t.year) || (t2.month == mo && t2.year == t.year);
            const bool this_mo  = t.month == mo;
            if (t1_or_t2) ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
            if (this_mo) ImGui::PushStyleColor(ImGuiCol_Button, col_hi);
            if (ImGui::Button(abrv_month_name(mo), cell_size) && !changed)
            {
                changed = true;
                t.month = mo;
                level   = 0;
            }
            if (t1_or_t2) ImGui::PopStyleColor();
            if (this_mo) ImGui::PopStyleColor();
            if (t1_or_t2) ImGui::PopStyleColor();
            if (mo % 4) ImGui::SameLine();
        }
    }
    // month widget
    else if (level == 2)
    {
        t.day   = 1;
        t.month = 1;
        int yr  = t.year - t.year % 20;
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        snprintf(buff, 32, "%d-%d", yr, yr + 19);
        ImGui::Button(buff);
        ImGui::PopItemFlag();
        ImGui::SameLine(5 * cell_size.x);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && yr - 20 > min_yr)
        {
            t.year -= 20;
            apply();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && yr + 20 < max_yr)
        {
            t.year += 20;
            apply();
        }
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f / 4.0f;
        cell_size.y *= 7.0f / 5.0f;
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                const bool t1_or_t2 = t1.year == yr || t2.year == yr;
                const bool cur_yr   = t.year == yr;
                if (t1_or_t2) ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
                if (cur_yr) ImGui::PushStyleColor(ImGuiCol_Button, col_hi);
                snprintf(buff, 32, "%d", yr);
                if (yr < 1970 || yr > 3000) { ImGui::Dummy(cell_size); }
                else if (ImGui::Button(buff, cell_size))
                {
                    t.year  = yr;
                    level   = 1;
                    changed = true;
                }
                if (cur_yr) ImGui::PopStyleColor();
                if (t1_or_t2) ImGui::PopStyleColor();
                if (j != 3) ImGui::SameLine();
                yr++;
            }
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::PopID();

    if (changed) apply();
    return changed;
}

bool imgui::day_time_picker(char const* id, imgui::time_point& to_edit)
{
    ImGui::PushID(id);

    static const char* nums[] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14",
                                 "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                                 "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44",
                                 "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};

    detail::ymdhms t       = detail::from_duration(to_edit.time_since_epoch());
    bool           changed = false;

    ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
    spacing.x      = 0;
    float width    = ImGui::CalcTextSize("888").x;
    float height   = ImGui::GetFrameHeight();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));

    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##hr", nums[t.hour], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 24; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.hour))
            {
                t.hour  = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##min", nums[t.minute], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 60; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.minute))
            {
                t.minute = i;
                changed  = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##sec", nums[t.second], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 60; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.second))
            {
                t.second = i;
                changed  = true;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopID();

    if (changed) to_edit = imgui::time_point{detail::to_duration(t)};

    return changed;
}
