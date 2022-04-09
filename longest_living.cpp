#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using Duration = std::chrono::milliseconds;
using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;
using Hash512 = std::array<uint8_t, 64>;

std::istream& getcsvfieldnoesc(std::istream& is, std::string& fld) {
	constexpr auto eof = std::remove_reference<decltype(is)>::type::traits_type::eof();
	int c;
	while (c = is.peek(), c != ',' && c != '\n' && c != eof) {
		if (c == '\r') {
			is.get();
			if (is.peek() == '\n') {
				return is;
			}
			fld += '\r';
		} else {
			fld += is.get();
		}
	}
	return is;
}

std::istream& getcsvfieldesc(std::istream& is, std::string& fld) {
	constexpr auto eof = std::remove_reference<decltype(is)>::type::traits_type::eof();
	assert(is.get() == '"');
	while (is.peek() != '"' || (is.get(), is.peek() == '"')) {
		fld += is.get();
	}
	int c = is.peek();
	if (c == ',' || c == '\n' || c == eof) {
		return is;
	} else if (c == '\r' && (is.get(), is.peek() == '\n')) {
		return is;
	} else {
		is.setstate(is.rdstate() | std::ios_base::failbit);
	}
	return is;
}

std::istream& getcsvfields(std::istream& is, std::string& head) {
	head.clear();
	if (is.peek() != '"') {
		getcsvfieldnoesc(is, head);
	} else {
		getcsvfieldesc(is, head);
	}
	return is;
}

template<typename... T>
std::istream& getcsvfields(std::istream& is, std::string& head, T&... tail) {
	getcsvfields(is, head);
	if (is.peek() == ',') {
		is.get();
	}
	getcsvfields(is, tail...);
	return is;
}

template<typename... T>
std::istream& getcsvline(std::istream& is, T&... args) {
	getcsvfields(is, args...);
	is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return is;
}

TimePoint utc_to_tp(const std::string& s) {
	std::istringstream ss(s);
	int y, m, d, h, min, sec, ms;
	(((((ss >> y).ignore(1, '-') >> m).ignore(1, '-') >> d >>
			h).ignore(1, ':') >> min).ignore(1, ':') >>
			sec).ignore(1, '.');
	ms = 0;
	for (int i = 0; i < 3; ++i) {
		ms *= 10;
		ms += ss.peek() == ' ' ? 0 : ss.get() - '0';
	}
	int jdn = (1461 * (y + 4800 + (m - 14) / 12)) / 4 + (367 * (m - 2 - 12 * ((m - 14) / 12))) / 12 - (3 * ((y + 4900 + (m - 14) / 12) / 100)) / 4 + d - 32075;
	uint64_t uts = (jdn - 2440588) * 86400 + h * 60 * 60 + min * 60 + sec;
	return TimePoint(Duration(uts * 1000 + ms));
}

Hash512 b64decode512bit(const std::string& s) {
	constexpr std::array<int8_t, 256> b64tobits = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, 63, 62, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};
	Hash512 ret = {};
	size_t n = s.size();
	int i = 0, j = 0;
	for (; i < n && b64tobits[s[i]] >= 0; ++i) {
		if (i % 4 != 3) {
			continue;
		}
		uint32_t bits = (b64tobits[s[i - 3]] << 18U) |
				(b64tobits[s[i - 2]] << 12U) |
				(b64tobits[s[i - 1]] << 6U) |
				b64tobits[s[i]];
		ret[j++] = bits >> 16;
		ret[j++] = (bits >> 8) & 0xff;
		ret[j++] = bits & 0xff;
	}
	switch(i % 3) {
		case 0:
			// Normal
		case 1:
			// Error
		break;
		case 2: {
			// 2 b64 chars -> 1 output byte
			uint32_t bits = (b64tobits[s[i - 2]] << 18U) |
					(b64tobits[s[i - 1]] << 12U);
			ret[j++] = bits >> 16;
		} break;
		case 3: {
			// 3 b64 chars -> 2 output bytes
			uint32_t bits = (b64tobits[s[i - 3]] << 18U) |
					(b64tobits[s[i - 2]] << 12U) |
					(b64tobits[s[i - 1]] << 6U);
			ret[j++] = bits >> 16;
			ret[j++] = (bits >> 8) & 0xff;
		} break;
	}
	return ret;
}

uint32_t colortobytes(const std::string& s) {
	std::istringstream ss(s);
	ss.ignore(1, '#');
	uint32_t ret;
	ss >> std::hex >> ret;
	return ret;
}

int main(int argc, char* argv[]) {
	constexpr unsigned canvas_h = 2000;
	constexpr unsigned canvas_w = 2000;
	constexpr TimePoint start_time = TimePoint(Duration(1648817027221));
	constexpr Hash512 start_uid = {};
	constexpr uint32_t start_color = 0xffffff;
	size_t n = 1000000;
	if (argc < 2) {
		std::clog << "Usage: " << argv[0] << " filename" << std::endl;
		return 1;
	}
	if (argc >= 3) {
		n = std::stoll(argv[2]);
	}
	std::ifstream infile(argv[1]);
	std::vector<std::array<std::tuple<TimePoint, Hash512, uint32_t>, canvas_w>> last_modified(canvas_h);
	std::vector<std::array<Hash512, canvas_w>> last_uid(canvas_h);
	std::priority_queue<std::tuple<Duration, TimePoint, Hash512, uint32_t, unsigned, unsigned>,
			std::vector<std::tuple<Duration, TimePoint, Hash512, uint32_t, unsigned, unsigned>>,
			std::greater<std::tuple<Duration, TimePoint, Hash512, uint32_t, unsigned, unsigned>>> longest_living;
	std::for_each(last_modified.begin(), last_modified.end(), [&](auto& row){std::fill(row.begin(), row.end(), std::make_tuple(start_time, start_uid, start_color));});
	std::string ts_s, uid_s, color_s, coords_s;
	const auto update_last_modified = [&](unsigned x, unsigned y) {
		const auto& [pts, puid, pcolor] = last_modified[y][x];
		const auto ts = utc_to_tp(ts_s);
		const auto uid = b64decode512bit(uid_s);
		const auto color = colortobytes(color_s);
		if (std::any_of(puid.begin(), puid.end(), [](auto x){return x;})) {
			longest_living.emplace(ts - pts, pts, puid, pcolor, x, y);
			if (longest_living.size() > n) {
				longest_living.pop();
			}
		}
		last_modified[y][x] = {ts, uid, color};
	};
	// Read header first
	getcsvline(infile, ts_s, uid_s, color_s, coords_s);
	while (getcsvline(infile, ts_s, uid_s, color_s, coords_s)) {
		std::istringstream coordss(coords_s);
		unsigned x1, y1, x2, y2;
		if ((((coordss >> x1).ignore(1, ',') >> y1).ignore(1, ',') >>
				x2).ignore(1, ',') >> y2) {
			for (unsigned x = x1; x <= x2; ++x) {
				for (unsigned y = y1; y <= y2; ++y) {
					update_last_modified(x, y);
				}
			}
		} else {
			update_last_modified(x1, y1);
		}
	}
	// Done, list out results
	while (!longest_living.empty()) {
		const auto& [duration, ts, uid, color, x, y] = longest_living.top();
		std::cout << std::dec << std::setw(0) << std::setfill(' ') << duration.count() << "," << ts.time_since_epoch().count() << ",";
		for (const auto& c : uid) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << +c;
		}
		std::cout << "," << std::hex << std::setw(6) << std::setfill('0') << color << "," << std::dec << std::setw(0) << std::setfill(' ') << x << "," << y << std::endl;
		longest_living.pop();
	}
}
