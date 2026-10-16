#include "Translate.h"
#include "ILanguage.h"

static ILanguage *s_pLanguage = NULL;

const wchar_t *Tr(const wchar_t *szEnglish) {
    return s_pLanguage ? s_pLanguage->translate(szEnglish) : szEnglish;
}

void SetUiLanguage(ILanguage *pLanguage) {
    s_pLanguage = pLanguage;
}

ILanguage *GetUiLanguage() {
    return s_pLanguage;
}
