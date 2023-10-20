#pragma once
#include <log.hpp>
#include <array>

namespace Language {

enum StringID {
	MENU_FILE,
	FILE_ITEM_OPEN,
	FILE_ITEM_EXIT,
	MENU_EMULATION,
	EMULATION_ITEM_RESET,
	EMULATION_ITEM_STOP,
	EMULATION_ITEM_PAUSE,
	EMULATION_ITEM_RESUME,
	EMULATION_ITEM_SETTINGS,
	SETTINGS_CATEGORY_CPU,
	SETTINGS_CATEGORY_AUDIO,
	SETTINGS_CATEGORY_INTERFACE,
	SETTINGS_OPTION_MUTE,
	SETTINGS_OPTION_VOLUME_L,
	SETTINGS_OPTION_VOLUME_R,
	SETTINGS_OPTION_LOCK_CHANNELS,
	SETTINGS_OPTION_ENABLE_JIT,
	SETTINGS_OPTION_LANGUAGE,
	LANGUAGE_ENGLISH,
	LANGUAGE_ITALIAN,
	STRING_COUNT
};

static const std::array<std::string, STRING_COUNT> english = {
	"File",
	"Open",
	"Exit",
	"Emulation",
	"Reset",
	"Stop",
	"Pause",
	"Resume",
	"Settings",
	"CPU",
	"Audio",
	"Interface",
	"Mute",
	"Volume L",
	"Volume R",
	"Lock channels",
	"Enable JIT",
	"Language"
};

static const std::array<std::string, STRING_COUNT> italian = {
	"File",
	"Apri",
	"Esci",
	"Emulazione",
	"Reset",
	"Stop",
	"Pausa",
	"Riprendi",
	"Impostazioni",
	"CPU",
	"Audio",
	"Interfaccia",
	"Muto",
	"Volume L",
	"Volume R",
	"Blocca canali",
	"Abilita JIT",
	"Lingua"
};

enum AvailableLangs {
	ENGLISH,
	ITALIAN,
	AVAILABLE_LANGS_COUNT
};

static const std::array<std::string, AVAILABLE_LANGS_COUNT> languages = {
	"English",
	"Italiano"
};

void SetLanguage(const std::array<std::string, STRING_COUNT>& lang, int selectedLang);
}