#include "LanguageFactory.h"

/* Language factory constructor.
Register the types of languages here.
 */
LanguageFactory::LanguageFactory() {
    // Register(“Horse”, &Horse::Create);
    // Register(“Cat”, &Cat::Create);
    Register("English", &English::Create);
    Register("Srpski", &Srpski::Create);
    Register("Srpski_Cyr", &Srpski_Cyr::Create);
    Register("Deutsch", &Deutsch::Create);
}

void LanguageFactory::Register(const std::string &languageName, CreateLanguageFn pfnCreate) {
    m_FactoryMap[languageName] = pfnCreate;
}

ILanguage *LanguageFactory::CreateLanguage(const std::string &languageName) {
    FactoryMap::iterator it = m_FactoryMap.find(languageName);
    if (it != m_FactoryMap.end())
        return it->second();
    return NULL;
}