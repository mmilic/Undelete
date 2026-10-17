/*
 * File:   ILanguage.h
 * Author: ezormil
 *
 * Created on 1. Juli 2014, 14:56
 *
 * A language of the UI: the translation of each English text (Translate.h). A language without a
 * translation for a text keeps the English one.
 */

#ifndef ILANGUAGE_H
#define	ILANGUAGE_H

class ILanguage {
public:

    virtual ~ILanguage() {
    };

    virtual const wchar_t *translate(const wchar_t *szEnglish) {
        return szEnglish;
    }

};

typedef ILanguage* (__stdcall *CreateLanguageFn)(void); //WIN32 specific, won't compile on any other OS

//Language classes declarations

class English : public ILanguage {
public:

    static ILanguage * __stdcall Create() {
        return new English();
    };
};

class Deutsch : public ILanguage {
public:

    static ILanguage * __stdcall Create() {
        return new Deutsch();
    };
    const wchar_t *translate(const wchar_t *szEnglish) override; // GermanTexts.cpp

};
#endif	/* ILANGUAGE_H */
