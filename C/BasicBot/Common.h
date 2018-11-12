#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <cstdio>
#include <cstdlib>

#include <stdarg.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <array>
#include <ctime>
#include <iomanip>

#include <winsock2.h>
#include <windows.h>

#include <BWAPI.h>
#include <BWTA.h>

#include "Config.h"
#include "CommandUtil.h"

namespace MyBot
{

	struct double2
	{
		double x, y;

		double2() {}
		double2(double x, double y) : x(x), y(y) {}
		double2(const BWAPI::Position & p) : x(p.x), y(p.y) {}

		operator BWAPI::Position()				const { return BWAPI::Position(static_cast<int>(x), static_cast<int>(y)); }

		double2 operator + (const double2 & v)	const { return double2(x + v.x, y + v.y); }
		double2 operator - (const double2 & v)	const { return double2(x - v.x, y - v.y); }
		double2 operator * (double s)			const { return double2(x*s, y*s); }
		double2 operator / (double s)			const { return double2(x / s, y / s); }

		double dot(const double2 & v)			const { return x*v.x + y*v.y; }
		double lenSq()							const { return x*x + y*y; }
		double len()							const { return sqrt(lenSq()); }
		double2 normal()						const { return *this / len(); }

		void normalise() { double s(len()); x /= s; y /= s; }
		void rotate(double angle)
		{
			angle = angle*M_PI / 180.0;
			*this = double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
		}
	};

	/// 로그 유틸
	namespace Logger
	{
		void appendTextToFile(const std::string & logFile, const std::string & msg);
		void appendTextToFile(const std::string & logFile, const char *fmt, ...);
		void overwriteToFile(const std::string & logFile, const std::string & msg);
	};

	/// 파일 유틸
	namespace FileUtil {
		/// 파일 유틸 - 텍스트 파일을 읽어들인다
		std::string readFile(const std::string & filename);

		/// 파일 유틸 - 경기 결과를 텍스트 파일로부터 읽어들인다
		void readResults();

		/// 파일 유틸 - 경기 결과를 텍스트 파일에 저장한다
		void writeResults();
	}	

}