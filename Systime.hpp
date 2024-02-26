#include <iostream>
#include <string>
#include <map>
#include <Windows.h>

const std::map<unsigned short,std::string> _dayOfWeek{{0, "Sun"}, {1, "Mon"}, {2, "Tue"}, {3, "Wed"}, {4, "Thu"}, {5, "Fri"}, {6, "Sat"}};
const std::map<unsigned short,std::string> _month{{1, "Jan"}, {2, "Feb"}, {3, "Mar"}, {4, "Apr"}, {5, "May"}, {6, "Jun"}, {7, "Jul"}, {8, "Aug"}, {9, "Sept"}, {10, "Oct"}, {11, "Nov"}, {12, "Dec"}};     

class Systime {
    public:
    SYSTEMTIME data;

    Systime(){}

    Systime(const SYSTEMTIME &data):data(data){}
    
    void operator ()(const SYSTEMTIME &data) { this->data=data;}

    SYSTEMTIME* operator &() { return &data; }

    friend std::ostream& operator<<(std::ostream& os, const Systime& obj)
    {
        auto dof=_dayOfWeek.begin();
        std::advance(dof,obj.data.wDayOfWeek);
        auto mon = _month.find(obj.data.wMonth);
        std::string sDate= (dof != _dayOfWeek.end() ? dof->second + " " : "") + std::to_string(obj.data.wDay) + " " + (mon != _month.end() ? mon->second + " " : "") + (obj.data.wHour<10 ? "0":"") +
        std::to_string(obj.data.wHour) + ":" + (obj.data.wMinute<10 ? "0":"") + std::to_string(obj.data.wMinute) + ":" + (obj.data.wSecond<10 ? "0":"") + std::to_string(obj.data.wSecond) + " " + std::to_string(obj.data.wYear);
        os<<sDate;
        return os;
    }

    void add(double seconds)
    {
        /* Convert SYSTEMTIME to FILETIME */ 
        FILETIME f;
        SystemTimeToFileTime( &data, &f );

        /* Convert FILETIME to ULARGE_INTEGER for arithmetic operations */
        ULARGE_INTEGER u; 
        u.LowPart = f.dwLowDateTime;
        u.HighPart = f.dwHighDateTime;    

        const double c_dSecondsPer100nsInterval = 100.*1.e-9;
        const double c_dNumberOf100nsIntervals =  seconds / c_dSecondsPer100nsInterval;

        /* Add x seconds (100 nanoseconds) to ULARGE_INTEGER */
        u.QuadPart += c_dNumberOf100nsIntervals;

        /* Convert ULARGE_INTEGER back to FILETIME */ 
        f.dwLowDateTime = u.LowPart;
        f.dwHighDateTime = u.HighPart;
        FileTimeToSystemTime( &f, &data );
    }
    

};