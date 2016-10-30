#ifndef __UTILS_HPP__
#define __UTILS_HPP__


#include <stddef.h>
#include <string>


double getTimeDouble();
size_t getTimestamp();


namespace path
{
	std::string basename(const std::string& path);
	std::string clearname(const std::string& path);
	std::string dirname(const std::string& path);
	std::string join(const std::string& path1, const std::string& path2);
}


#endif // __UTILS_HPP__