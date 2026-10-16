/*
 * File:   Translate.h
 *
 * The text of the UI in the chosen language. The English text is the key: Tr(L"Back") is "Zurück" in German,
 * and a language that has no translation for a text shows the English one - a missing entry never shows an
 * empty button. The language packs (LanguagesImpl.cpp) hold the tables.
 */

#ifndef TRANSLATE_H
#define	TRANSLATE_H

#include <wx/string.h>

class ILanguage;

const wchar_t *Tr(const wchar_t *szEnglish);

// for a text that isn't a literal - e.g. a label the engine returns
inline wxString Tr(const wxString &sEnglish) {
    return wxString(Tr(sEnglish.wc_str()));
}

// the language of the whole UI - the pack stays owned by the caller (it lives as long as the app)
void SetUiLanguage(ILanguage *pLanguage);
ILanguage *GetUiLanguage();

#endif	/* TRANSLATE_H */
