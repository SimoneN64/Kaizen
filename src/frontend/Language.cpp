#include <Language.hpp>

namespace Language {
void SetLanguage(std::array<std::string, STRING_COUNT>& lang, int selectedLang) {
    switch (selectedLang) {
	case AvailableLangs::ENGLISH: lang = english; break;
	case AvailableLangs::ITALIAN: lang = italian; break;
	default: Util::panic("Language not supported, index {}\n", selectedLang);
	}
}
}