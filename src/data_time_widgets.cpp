#include "imgui/widgets.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

// C++20 backport :(
namespace imgui
{
using years    = std::chrono::duration<int32_t, std::ratio<31556952>>;
using months   = std::chrono::duration<int32_t, std::ratio<2629746>>;
using days     = std::chrono::duration<int32_t, std::ratio<86400>>;
using hours    = std::chrono::hours;
using minutes  = std::chrono::minutes;
using seconds  = std::chrono::seconds;
using duration = time_point::duration;
namespace detail
{
struct ymdhms
{
    years   y_;
    months  m_;
    days    d_;
    hours   h_;
    minutes mins_;
    seconds s_;
    ymdhms(duration d)
        : y_{std::chrono::duration_cast<years>(abs(d))},
          m_{std::chrono::duration_cast<months>(abs(d) - y_)},
          d_{std::chrono::duration_cast<days>(abs(d) - y_ - m_)},
          h_{std::chrono::duration_cast<hours>(abs(d) - y_ - m_ - d_)},
          mins_{std::chrono::duration_cast<minutes>(abs(d) - y_ - m_ - d_ - h_)},
          s_{std::chrono::duration_cast<seconds>(abs(d) - y_ - m_ - d_ - h_ - mins_)}
    {
    }
    int             year() const { return y_.count() + 1970; }
    int             month() const { return m_.count(); }
    int             day() const { return d_.count(); }
    int             hour() const { return h_.count(); }
    int             min() const { return mins_.count(); }
    int             sec() const { return s_.count(); }
    inline duration to_duration() const { return std::chrono::duration_cast<duration>(y_ + m_ + d_ + h_ + mins_ + s_); }
};

// Returns true if year is leap year (366 days long)
inline bool is_leap_year(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }
// Returns the number of days in a month, accounting for Feb. leap years. #month is zero indexed.
inline int days_in_month(int year, int month)
{
    static const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month] + (int)(month == 1 && is_leap_year(year));
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

    static const char* names_mo[] = {"January", "February", "March",     "April",   "May",      "June",
                                     "July",    "August",   "September", "October", "November", "December"};
    static const char* abrvs_mo[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char* abrvs_wd[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    ImGuiStyle&    style   = ImGui::GetStyle();
    ImVec4         col_txt = style.Colors[ImGuiCol_Text];
    ImVec4         col_dis = style.Colors[ImGuiCol_TextDisabled];
    ImVec4         col_hi  = style.Colors[ImGuiCol_ButtonActive];
    const float    ht      = ImGui::GetFrameHeight();
    ImVec2         cell_size(ht * 1.25f, ht);
    char           buff[32];
    bool           changed = false;
    detail::ymdhms t1{min_time.time_since_epoch()};
    detail::ymdhms t2{max_time.time_since_epoch()};
    detail::ymdhms t{to_edit.time_since_epoch()};
    auto           apply = [&to_edit, &t]() { to_edit = imgui::time_point{t.to_duration()}; };

    const int min_yr = 1970;
    const int max_yr = 2999;

    // day widget
    if (level == 0)
    {
        const int this_year    = t.year();
        const int last_year    = this_year - 1;
        const int next_year    = this_year + 1;
        const int this_mon     = t.month();
        const int last_mon     = this_mon == 0 ? 11 : this_mon - 1;
        const int next_mon     = this_mon == 11 ? 0 : this_mon + 1;
        const int days_this_mo = detail::days_in_month(this_year, this_mon);
        const int days_last_mo = detail::days_in_month(this_mon == 0 ? last_year : this_year, last_mon);
        auto      first_mo     = std::chrono::system_clock::to_time_t(time_point{std::chrono::floor<months>(t.to_duration())});
        std::tm   ts;
#ifdef WIN32
        localtime_s(&ts, &first_mo);
#else
        localtime_r(&first_mo, &ts);
#endif
        const int first_wd = ts.tm_wday;
        // month year
        snprintf(buff, 32, "%s %d", names_mo[this_mon], this_year);
        if (ImGui::Button(buff)) level = 1;
        ImGui::SameLine(5 * cell_size.x);
        auto enable_month_down = !(this_year <= min_yr && this_mon == 0);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && enable_month_down)
        {
            t.m_ -= months{1};
            apply();
        }
        ImGui::SameLine();
        auto enable_month_up = !(this_year >= max_yr && this_mon == 11);
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && enable_month_up)
        {
            t.m_ += months{1};
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
                const int now_yr = (mo == 0 && this_mon == 0) ? last_year : ((mo == 2 && this_mon == 11) ? next_year : this_year);
                const int now_mo = mo == 0 ? last_mon : (mo == 1 ? this_mon : next_mon);
                const int now_md = day;

                const bool off_mo   = mo == 0 || mo == 2;
                const bool t1_or_t2 = (t1.month() == now_mo && t1.year() == now_yr && t1.day() == now_md) ||
                                      (t2.month() == now_mo && t2.year() == now_yr && t2.day() == now_md);

                if (off_mo) ImGui::PushStyleColor(ImGuiCol_Text, col_dis);
                if (t1_or_t2)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
                    ImGui::PushStyleColor(ImGuiCol_Text, col_txt);
                }
                auto on_selected_day = mo == 1 && days{day} == t.d_;
                if (on_selected_day)
                    ImGui::PushStyleColor(ImGuiCol_Button, col_hi);
                ImGui::PushID(i * 7 + j);
                snprintf(buff, 32, "%d", day);
                if (now_yr == min_yr - 1 || now_yr == max_yr + 1) { ImGui::Dummy(cell_size); }
                else if (ImGui::Button(buff, cell_size) && !changed)
                {
                    t.y_    = years{now_yr - 1970};
                    t.m_    = months{now_mo};
                    t.d_    = days{now_md};
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
        t = detail::ymdhms{time_point{std::chrono::floor<months>(t.to_duration())}.time_since_epoch()};
        snprintf(buff, 32, "%d", t.year());
        if (ImGui::Button(buff)) level = 2;
        ImGui::SameLine(5 * cell_size.x);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && t.year() > min_yr)
        {
            t.y_ -= years{1};

            apply();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && t.year() < max_yr)
        {
            t.y_ += years{1};
            apply();
        }
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f / 4.0f;
        cell_size.y *= 7.0f / 3.0f;
        int mo = 0;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                const bool t1_or_t2 = (t1.month() == mo && t1.year() == t.year()) || (t2.month() == mo && t2.year() == t.year());
                if (t1_or_t2) ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
                if (ImGui::Button(abrvs_mo[mo], cell_size) && !changed)
                {
                    changed = true;
                    t.m_    = months{mo};
                    level   = 0;
                }
                if (t1_or_t2) ImGui::PopStyleColor();
                if (j != 3) ImGui::SameLine();
                mo++;
            }
        }
    }
    // month widget
    else if (level == 2)
    {
        t           = detail::ymdhms{time_point{std::chrono::floor<years>(t.to_duration())}.time_since_epoch()};
        int this_yr = t.year();
        int yr      = this_yr - this_yr % 20;
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        snprintf(buff, 32, "%d-%d", yr, yr + 19);
        ImGui::Button(buff);
        ImGui::PopItemFlag();
        ImGui::SameLine(5 * cell_size.x);
        if (ImGui::ArrowButtonEx("##Up", ImGuiDir_Up, cell_size, ImGuiButtonFlags_None) && yr - 20 > min_yr)
        {
            t.y_ -= years{20};
            apply();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButtonEx("##Down", ImGuiDir_Down, cell_size, ImGuiButtonFlags_None) && yr + 20 < max_yr)
        {
            t.y_ += years{20};
            apply();
        }
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f / 4.0f;
        cell_size.y *= 7.0f / 5.0f;
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                const bool t1_or_t2 = t1.year() == yr || t2.year() == yr;
                if (t1_or_t2) ImGui::PushStyleColor(ImGuiCol_Button, col_dis);
                snprintf(buff, 32, "%d", yr);
                if (yr < 1970 || yr > 3000) { ImGui::Dummy(cell_size); }
                else if (ImGui::Button(buff, cell_size))
                {
                    t.y_    = years{this_yr - 1970};
                    level   = 1;
                    changed = true;
                }
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

    detail::ymdhms t(to_edit.time_since_epoch());
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
    if (ImGui::BeginCombo("##hr", nums[t.h_.count()], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 24; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.h_.count()))
            {
                t.h_    = hours{i};
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##min", nums[t.mins_.count()], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 60; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.mins_.count()))
            {
                t.mins_ = minutes{i};
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##sec", nums[t.s_.count()], ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < 60; ++i)
        {
            if (ImGui::Selectable(nums[i], i == t.s_.count()))
            {
                t.s_    = seconds{i};
                changed = true;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopID();

    if (changed) to_edit = imgui::time_point{t.to_duration()};

    return changed;
}
