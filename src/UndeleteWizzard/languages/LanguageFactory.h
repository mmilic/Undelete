/* 
 * File:   LanguageFactory.h
 * Author: ezormil
 *
 * Created on 1. Juli 2014, 15:42
 */

#ifndef LANGUAGEFACTORY_H
#define	LANGUAGEFACTORY_H

#include <map>
#include <iostream>
#include <string>
#include "ILanguage.h"




// Factory for creating instances of ILanguage

class LanguageFactory {
private:
    LanguageFactory();

    LanguageFactory(const LanguageFactory &) {
    }

    LanguageFactory &operator=(const LanguageFactory &) {
        return *this;
    }

    typedef std::map<std::string, CreateLanguageFn> FactoryMap;
    FactoryMap m_FactoryMap;
public:

    ~LanguageFactory() {
        m_FactoryMap.clear();
    }

    static LanguageFactory *Get() {
        static LanguageFactory instance;
        return &instance;
    }

    void Register(const std::string &languageName, CreateLanguageFn pfnCreate);
    ILanguage *CreateLanguage(const std::string &languageName);
};

#endif	/* LANGUAGEFACTORY_H */