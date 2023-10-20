#include <Language.hpp>

namespace Language {
void SetLanguage(const std::array<std::string, STRING_COUNT>& lang, int selectedLang) {
	switch (selectedLang) {
	case AvailableLangs::ENGLISH: std::copy(english.begin(), english.end(), lang.begin()); break;
	case AvailableLangs::ITALIAN: std::copy(italian.begin(), italian.end(), lang.begin()); break;
	default: Util::panic("Language not supported, index {}\n", selectedLang);
	}
}
}