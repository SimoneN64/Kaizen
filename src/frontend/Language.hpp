#pragma once
#include <log.hpp>
#include <map>
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
	SETTINGS_CLOSE,
	STRING_COUNT
};

static const std::map <StringID, const char*> english = {
	{MENU_FILE, "File"},
	{FILE_ITEM_OPEN, "Open"},
	{FILE_ITEM_EXIT, "Exit"},
	{MENU_EMULATION, "Emulation"},
	{EMULATION_ITEM_RESET, "Reset"},
	{EMULATION_ITEM_STOP, "Stop"},
	{EMULATION_ITEM_PAUSE, "Pause"},
	{EMULATION_ITEM_RESUME, "Resume"},
	{EMULATION_ITEM_SETTINGS, "Settings"},
	{SETTINGS_CATEGORY_CPU, "CPU"},
	{SETTINGS_CATEGORY_AUDIO, "Audio"},
	{SETTINGS_CATEGORY_INTERFACE, "Interface"},
	{SETTINGS_OPTION_MUTE, "Mute"},
	{SETTINGS_OPTION_VOLUME_L, "Volume L"},
	{SETTINGS_OPTION_VOLUME_R, "Volume R"},
	{SETTINGS_OPTION_LOCK_CHANNELS, "Lock channels"},
	{SETTINGS_OPTION_ENABLE_JIT, "Enable JIT"},
	{SETTINGS_OPTION_LANGUAGE, "Language"},
	{SETTINGS_CLOSE, "Close"}
};

static const std::map <StringID, const char*> italian = {
	{MENU_FILE, "File"},
	{FILE_ITEM_OPEN, "Apri"},
	{FILE_ITEM_EXIT, "Esci"},
	{MENU_EMULATION, "Emulazione"},
	{EMULATION_ITEM_RESET, "Reset"},
	{EMULATION_ITEM_STOP, "Stop"},
	{EMULATION_ITEM_PAUSE, "Pausa"},
	{EMULATION_ITEM_RESUME, "Riprendi"},
	{EMULATION_ITEM_SETTINGS, "Opzioni"},
	{SETTINGS_CATEGORY_CPU, "CPU"},
	{SETTINGS_CATEGORY_AUDIO, "Audio"},
	{SETTINGS_CATEGORY_INTERFACE, "Interfaccia"},
	{SETTINGS_OPTION_MUTE, "Muta"},
	{SETTINGS_OPTION_VOLUME_L, "Volume L"},
	{SETTINGS_OPTION_VOLUME_R, "Volume R"},
	{SETTINGS_OPTION_LOCK_CHANNELS, "Blocca canali"},
	{SETTINGS_OPTION_ENABLE_JIT, "Abilita JIT"},
	{SETTINGS_OPTION_LANGUAGE, "Lingua"},
	{SETTINGS_CLOSE, "Chiudi"}
};

enum AvailableLangs {
	ENGLISH,
	ITALIAN,
	AVAILABLE_LANGS_COUNT
};

static const std::array<const char*, AVAILABLE_LANGS_COUNT> languages = {
	"English",
	"Italiano"
};

static FORCE_INLINE void SetLanguage(std::map<StringID, const char*>& lang, int selectedLang) {
  switch (selectedLang) {
		case AvailableLangs::ENGLISH: lang = english; break;
		case AvailableLangs::ITALIAN: lang = italian; break;
		default: Util::panic("Language not supported, index {}\n", selectedLang);
	}
}
}