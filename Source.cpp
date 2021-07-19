#include "filter_view.h"
#include <vector>
#include <iostream>
int main() {
	std::vector<int> numbers{ 1,2,3,4,5,6,7,8,9 };
	auto isEven = [](auto number) {return number % 2 == 0; };
	for (auto elem : views::filter(numbers, isEven))
		std::cout << elem << " ";
}