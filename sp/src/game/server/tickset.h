#pragma once

#ifdef _WIN32

class TickSet {
public:
	static bool TickInit();
	static bool SetTickrate(float);

private:
	static inline bool DataCompare(const unsigned char*, const unsigned char*, const char*);
	static void* FindPattern(const void*, size_t, const unsigned char*, const char*);
	static float* interval_per_tick;
};

#endif // _WIN32
