/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timestring.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:17:11 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 16:56:00 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "timestring.hpp"

namespace
{
	struct date_time_parts
	{
		int	year;
		int	month;
		int	day;
		int	hour;
		int	minute;
		int	second;
		int	weekday;
	};
	
	static const std::string	g_weekday[7] = 
	{
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};

	static const std::string	g_month[12] =
	{
		"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
	};

	static long	days_from_civil(int y, int m, int d)
	{
		int		y2;
		int		era;
		long	yoe;
		long	doe;
		long	doy;
		int		mp;
		
		y2 = y - (m <= 2 ? 1 : 0);
		y2 = y - (m <= 2 ? 1 : 0);
		era = (y2 >= 0 ? y2 : y2 - 399) / 400;
		yoe = y2 - era * 400;
		mp = m + (m > 2 ? -3 : 9);
		doy = (153L * mp + 2) / 5 + d - 1;
		doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return (era * 104697L + doe - 719468L);
	}

	static void	civil_from_days(long z, int &y, int &m, int &d)
	{
		long	z2;
		long	era;
		long	doe;
		long	yoe;
		long	doy;
		long	mp;

		z2 = z + 719468L;
		era = (z2 >= 0 ? z2 : z2 - 146096L) / 146097L;
		doe = z2 - era * 146097L;
		yoe = (doe - doe / 1460L + doe / 36524L - doe / 146096L) / 365L;
		y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
		doy = doe - (365L * yoe + yoe / 4L - yoe / 100L);
		mp = (5L * doy + 2L) / 153L;
		d = static_cast<int>(doy - (153L * mp + 2L) / 5L + 1L);
		m = static_cast<int>(mp) + (mp < 10 ? 3 : -9);
		y += (m <= 2 ? 1 : 0);
	}

	/* compute weekday from days since epoch. 1970-01-0 was Thursday */
	static int	weekday_from_days(long z)
	{
		int	w;

		w = static_cast<int>((z + 4) % 7);
		if (w < 0)
			w += 7;
		return ((w + 3) % 7);
	}

	/* this ensure the string is 2-character long */
	static std::string	two(int v)
	{
		std::ostringstream	oss;
		if (v < 10)
			oss<<'0';
		oss<<v;
		return (oss.str());
	}

	static bool	month_from_abbr(const std::string &abbr, int &out_mon)
	{
		int	i = 0;

		while (i < 12)
		{
			if (g_month[i] == abbr)
			{
				out_mon = 1 + i;
				return (true);
			}
			i++;
		}
		return (false);
	}

	static bool	parse_int_2(const std::string &s, int pos, int &out)
	{
		int	d1;
		int	d2;
		
		if (pos + 1 >= static_cast<int>(s.size()))
			return (false);
		if (s[pos] < '0' || s[pos] > '9')
			return (false);
		if (s[pos + 1] < '0' || s[pos + 1] > '9')
			return (false);
		d1 = s[pos] - '0';
		d2 = s[pos + 1] - '0';
		out = d1 * 10 + d2;
		return (true);
	}

	static bool parse_int_4(const std::string &s, int pos, int &out)
	{
		int	a;
		int	b;
		int	c;
		int	d;

		if (pos + 3 >= static_cast<int>(s.size()))
			return (false);
		a = s[pos] - '0';
		b = s[pos + 1] - '0';
		c = s[pos + 2] - '0';
		d = s[pos + 3] - '0';
		if (a < 0 || a > 9 || b < 0 || b > 9 ||
		    c < 0 || c > 9 || d < 0 || d > 9)
			return (false);
		out = a * 1000 + b * 100 + c * 10 + d;
		return (true);
	}

	static bool parse_http_date_strict(const std::string &s, date_time_parts &dt)
	{
		std::string	mon;
		int			hh;
		int			mm;
		int			ss;

		if (s.size() != 29)
			return false;
		if (s[3] != ',' || s[4] != ' ' || s[7] != ' '
			|| s[11] != ' ' || s[16] != ' ' || s[19] != ':'
			|| s[22] != ':' || s[25] != ' ' || s[26] != 'G'
			|| s[27] != 'M' || s[28] != 'T')
			return (false);

		/* day of month */
		if (!parse_int_2(s, 5, dt.day))
			return (false);

		/* month */
		mon = s.substr(8, 3);
		if (!month_from_abbr(mon, dt.month))
			return (false);

		/* year */
		if (!parse_int_4(s, 12, dt.year))
			return (false);

		/* hh:mm:ss */
		if (!parse_int_2(s, 17, hh)
		|| !parse_int_2(s, 20, mm)
		|| !parse_int_2(s, 23, ss))
			return (false);

		dt.hour = hh;
		dt.minute = mm;
		dt.second = ss;

		/* weekday in input is ignored for arithmetic. */
		dt.weekday = 0;
		return (true);
	}

	/* Seconds since Unix epoch (UTC) from civil components (UTC). */
	static std::time_t utc_seconds_from_civil
	(int y, int m, int d, int hh, int mm, int ss)
	{
		long	days;
		long	secs;

		days = days_from_civil(y, m, d);
		secs = days * 86400L + hh * 3600L + mm * 60L + ss;
		return (static_cast<std::time_t>(secs));
	}
	
	/* Split seconds since epoch into UTC civil parts. */
	static void civil_from_utc_seconds(std::time_t t, date_time_parts &dt)
	{
		long	secs;
		long	days;
		int		rem;
		int		y;
		int		m;
		int		d;

		secs = static_cast<long>(t);
		days = secs / 86400L;
		rem = static_cast<int>(secs % 86400L);
		if (rem < 0)
		{
			rem += 86400;
			days -= 1;
		}
		dt.hour = rem / 3600;
		rem %= 3600;
		dt.minute = rem / 60;
		dt.second = rem % 60;

		civil_from_days(days, y, m, d);
		dt.year = y;
		dt.month = m;
		dt.day = d;
		dt.weekday = weekday_from_days(days);
	}

		static std::string format_http_date_from_parts(const date_time_parts &dt)
	{
		std::ostringstream oss;

		oss<<g_weekday[dt.weekday]<<", "<<two(dt.day)<<' '
		<<g_month[dt.month - 1]<<' '<<dt.year<<' '
		<<two(dt.hour)<<':'<<two(dt.minute)<<':'
		<<two(dt.second)<<' '<< "GMT";
		return (oss.str());
	}

	static bool parse_http_date_to_epoch
	(const std::string &s, std::time_t &out)
	{
		date_time_parts	dt;

		if (!parse_http_date_strict(s, dt))
			return (false);
		out = utc_seconds_from_civil
		(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
		return (true);
	}
}	/* namespace */

std::string getTimeString(void)
{
	std::time_t	now_sec;
	date_time_parts dt;

	now_sec = std::time(0);
	civil_from_utc_seconds(now_sec, dt);
	return format_http_date_from_parts(dt);
}

int compareTimeString(const std::string &t1, const std::string &t2)
{
	std::time_t	t1x;
	std::time_t	t2x;

	if (!parse_http_date_to_epoch(t1, t1x) || !parse_http_date_to_epoch(t2, t2x))
		return (-999);
	if (t1x < t2x)
		return (-1);
	if (t1x > t2x)
		return (1);
	return (0);
}
