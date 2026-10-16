/* 
 * File:   ILanguage.h
 * Author: ezormil
 *
 * Created on 1. Juli 2014, 14:56
 */

#ifndef ILANGUAGE_H
#define	ILANGUAGE_H


#include <wx/stattext.h>
#include <iostream>

class ILanguage {
public:

    virtual ~ILanguage() {
    };
    virtual const wchar_t * getWXText(int page, int text) = 0;

    // the UI's text by its English wording (Translate.h) - a language without a translation keeps the English
    virtual const wchar_t *translate(const wchar_t *szEnglish) {
        return szEnglish;
    }

};

typedef ILanguage* (__stdcall *CreateLanguageFn)(void); //WIN32 specific, won't compile on any other OS
//typedef ILanguage* (*CreateLanguageFn)(void);

//Language classes declarations

class English : public ILanguage {
public:

    ~English() {
    };

    static ILanguage * __stdcall Create() {
        return new English();
    };

    const wchar_t *getWXText(int page, int text);

};

class Srpski : public ILanguage {
public:

    ~Srpski() {
    };

    static ILanguage * __stdcall Create() {
        return new Srpski();
    };
    const wchar_t *getWXText(int page, int text);

};
        
class Srpski_Cyr : public ILanguage {
public:

    ~Srpski_Cyr() {
    };

    static ILanguage * __stdcall Create() {
        return new Srpski_Cyr();
    };
    const wchar_t *getWXText(int page, int text);

};        

class Deutsch : public ILanguage {
public:

    ~Deutsch() {
    };

    static ILanguage * __stdcall Create() {
        return new Deutsch();
    };
    const wchar_t *getWXText(int page, int text);
    const wchar_t *translate(const wchar_t *szEnglish) override; // GermanTexts.cpp

};
#endif	/* ILANGUAGE_H */