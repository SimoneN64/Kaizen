#include <IR.hpp>

namespace n64 {
void IR::push(const Entry& e) {
	code.push_back(e);
}

auto IR::begin() {
	return code.begin();
}

auto IR::end() {
	return code.end();
}
}