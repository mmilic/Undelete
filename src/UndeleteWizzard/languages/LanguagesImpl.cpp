#include "ILanguage.h"
#include <iostream>
#include "LanguageEnums.h"

// ILanguage implementations
//    static ILanguage * __stdcall Create() { return new English(); }  //WIN32 specific, won't compile on any other OS

// how to change the texst on the buttons:
// http://trac.wxwidgets.org/ticket/5074

const wchar_t *English::getWXText(int page, int text) {

    switch (page) {
        case GENERAL:
            switch (text) {
                case ERROR_LABEL: return wxT("Error");
                    break;
                case NEXT_BUTTON_LABEL: return wxT("Next >");
                    break;
                case CANCEL_BUTTON_LABEL: return wxT("Cancel");
                    break;
                case BACK_BUTTON_LABEL: return wxT("< Back");
                    break;
                case RETRIEVE_BUTTON_LABEL: return wxT("&Retrieve >");
                    break;
                case BROWSE_BUTTON_LABEL: return wxT("Browse");
                    break;
                case FINISH_BUTTON_LABEL: return wxT("Finish");
                    break;
                case PROGRESS_DIALOG_QUESTION: return wxT("Progress dialog question");
                    break;
                case FILE_NAME: return wxT("File name");
                    break;
                case PROGRESS_DIALOG_ABORTED: return wxT("Progress dialog aborted!");
                    break;
                default: return wxT(" ");
            }
        case GREETINGS_PAGE:
            switch (text) {
                case WELCOME: return wxT("Welcome to the");
                    break;
                case WINDOWS_UNDELETE_WIZARD: return wxT("Windows Undelete Wizard");
                    break;
                case THIS_WIZARD_WILL_GUIDE_YOU: return
                    wxT("This Wizard will guide")
                    wxT("through the 3-step ")
                    wxT("Undelete Process\n");
                    break;
                case THE_UNDELETE_PROCESS_CONSISTS: return
                    wxT("The undelete process consists of 3 simple steps:\n\n")
                    wxT("- Step 1: Select the source drive from which you want to recover deleted files.\n\n")
                    wxT("- Step 2: Select the destination folder.\n")
                    wxT("               This is where GetMyFilesBack will save recovered files.\n")
                    wxT("               This folder mustn't be on the source drive.\n\n")
                    wxT("- Step 3: Select the files which you want to recover.\n\n\n\n");
                    break;
                case CLICK_NEXT_TO_CONTINUE: return wxT("Click 'Next' to continue\n");
                default: return wxT(" ");
                    break;
            }
            break;
        case SELECT_SOURCE_DRIVE_PAGE:
            switch (text) {
                case PLEASE_SELECT_SOURCE: return
                    wxT("Please select the source drive, ")
                    wxT("where you have deleted files and ")
                    wxT("click \'Next\'\n");
                    break;
                case PLEASE_SELECT_ONE_DRIVE: return wxT("Please select one drive!");
                    break;
                case ERROR_LOADING_FILES: return wxT("Error loading files\nCheck Access rights!");
                    break;
                default: return wxT(" ");
            }
        case SELECT_DESTINATION_PAGE:
            switch (text) {
                case PLEASE_SELECT_DESTINATION: return
                    wxT("Please select the destination ")
                    wxT("for the retrieved files and ")
                    wxT("click \'Next\'");
                    break;
                case SAME_DRIVE_ERROR: return
                    wxT("The folder you've selected is on the same drive as the files that need to be retrieved\n")
                    wxT("Please enter a folder on a different drive!");
                    break;
                case CANT_CREATE_THREADS_ERROR: return wxT("Can't create thread for reading the drive.");
                    break;
                case DO_YOU_REALLY_WANT_TO_CANCEL: return wxT("Do you really want to cancel ?");
                    break;

                default: return wxT(" ");
            }
        case SELECT_FILES_PAGE:
            switch (text) {
                case SELECT_ALL_BUTTON_LABEL: return wxT("Select All");
                    break;
                case UNSELECT_ALL_BUTTON_LABEL: return wxT("Unselect All");
                    break;
                case ALL_IMAGES_BUTTON_LABEL: return wxT("All Images");
                    break;
                case ALL_VIDEOS_BUTTON_LABEL: return wxT("All Videos");
                    break;
                case ALL_MUSIC_BUTTON_LABEL: return wxT("All Music");
                    break;
                case SELECT_FILES: return
                    wxT("Select the files you want to try to retrieve.\n\n")
                    wxT("To select multiple files, hold Ctrl\n\n.");
                    break;
                case SELECT_FILES_OTHER: return
                    wxT("Found %d files.\n")
                    wxT("Select the files you want to try to retrieve and click \'Retrieve\'\n\n")
                    wxT("To select multiple files, hold Ctrl.");
                    break;
                default: return wxT(" ");
            }
        case END_PAGE:
            switch (text) {
                case SUCCESSFULLY_RETRIEVED: return
                    wxT("Successfully retrieved %d files out of %d selected\n\n\n")
                    wxT("You can find them at\n%s.");
            }
        default: return wxT(" ");
    }
}

const wchar_t *Srpski::getWXText(int page, int text) {

    switch (page) {
        case GENERAL:
            switch (text) {
                case ERROR_LABEL: return wxT("Greška");
                    break;
                case NEXT_BUTTON_LABEL: return wxT("Dalje >");
                    break;
                case CANCEL_BUTTON_LABEL: return wxT("Otkaži");
                    break;
                case BACK_BUTTON_LABEL: return wxT("< Prethodna");
                    break;
                case RETRIEVE_BUTTON_LABEL: return wxT("&Od-Obriši");
                    break;
                case BROWSE_BUTTON_LABEL: return wxT("Pronađi");
                    break;
                case FINISH_BUTTON_LABEL: return wxT("Završi");
                    break;
                case PROGRESS_DIALOG_QUESTION: return wxT("Pitanje pretraživača");
                    break;
                case FILE_NAME: return wxT("Ime fajla");
                    break;
                case PROGRESS_DIALOG_ABORTED: return wxT("Pretraživač prekinut!");
                    break;
                default: return wxT(" ");
            }
        case GREETINGS_PAGE:
            switch (text) {
                case WELCOME: return wxT("Dobro nam došli u");
                    break;
                case WINDOWS_UNDELETE_WIZARD: return wxT("OD-OBRIŠI ZA WINDOWS");
                    break;
                case THIS_WIZARD_WILL_GUIDE_YOU: return
                    wxT(" Ovaj pomoćnik će vas provesti")
                    wxT(" kroz 3 koraka ")
                    wxT(" postupka od-obrisavanja\n");
                    break;
                case THE_UNDELETE_PROCESS_CONSISTS: return
                    wxT("Postupak od-obrisavanj se sastoji od 3 jednostavna koraka:\n\n")
                    wxT("- Korak 1: Označite disk sa koga želite da od-obrišete obrisane fajlove.\n\n")
                    wxT("- Korak 2: Označite željenu odredišnu fasciklu.\n")
                    wxT("               U nju će GetMyFilesBack snimiti od-obrisane fajlove.\n")
                    wxT("               Ta fascikla ne sme biti na disku navedenom u koraku 1.\n\n")
                    wxT("- Korak 3: Označite fajlove koje želite da od-obrišete.\n\n\n\n");
                    break;
                case CLICK_NEXT_TO_CONTINUE: return wxT("Kliknite na 'Dalje' da nastavite.\n");
                default: return wxT(" ");
                    break;
            }
            break;
        case SELECT_SOURCE_DRIVE_PAGE:
            switch (text) {
                case PLEASE_SELECT_SOURCE: return
                    wxT("Označite disk na kome ste obrisali fajlove i kliknite na 'Dalje' ");
                    break;
                case PLEASE_SELECT_ONE_DRIVE: return wxT("Molim selektujte jedan disk!");
                    break;
                case ERROR_LOADING_FILES: return wxT("Problem pri učitavanju fajlova\nProverita prava pristupa!");
                    break;
                default: return wxT(" ");
            }
        case SELECT_DESTINATION_PAGE:
            switch (text) {
                case PLEASE_SELECT_DESTINATION: return
                    wxT("Molim označite odredište ")
                    wxT("za od obrisane fajlove i ")
                    wxT("kliknite na  \'Dalje\'");
                    break;
                case SAME_DRIVE_ERROR: return
                    wxT("Fascikla koju ste oznčili se nalazi na istom disku kao i fajlovi koje želite da od-obrišete\n")
                    wxT("Molim odaberite fasciklu na drugom disku!");
                    break;
                case CANT_CREATE_THREADS_ERROR: return wxT("Ne mogu da napravim nit za čitanje diska.");
                    break;
                case DO_YOU_REALLY_WANT_TO_CANCEL: return wxT("Da li zaista želite da odustanete?");
                    break;

                default: return wxT(" ");
            }
        case SELECT_FILES_PAGE:
            switch (text) {
                case SELECT_ALL_BUTTON_LABEL: return wxT("Označi sve");
                    break;
                case UNSELECT_ALL_BUTTON_LABEL: return wxT("Od-označi sve");
                    break;
                case ALL_IMAGES_BUTTON_LABEL: return wxT("Sve slike");
                    break;
                case ALL_VIDEOS_BUTTON_LABEL: return wxT("Sаv video");
                    break;
                case ALL_MUSIC_BUTTON_LABEL: return wxT("Svu мuziku");
                    break;
                case SELECT_FILES: return
                    wxT("Označite fajlove koje želite da pokušate da od-obrišete.\n\n")
                    wxT("Da bi ste označili više fajlove, držite pritisnuto Ctrl\n\n.");
                    break;
                case SELECT_FILES_OTHER: return
                    wxT("Pronašao %d fajlova.\n")
                    wxT("Označite fajlove koje želite da pokušate da od-obrišete i kliknite na \'Od-Obriši\'\n\n")
                    wxT("Da bi ste označili više fajlovа, stisnite i držite tester Ctrl.");
                    break;
                default: return wxT(" ");
            }
        case END_PAGE:
            switch (text) {
                case SUCCESSFULLY_RETRIEVED: return
                    wxT("Uspešno od-obrisano %d fajlova od %d označena\n\n\n")
                    wxT("Možete ih pronaći u\n%s.");
            }
        default: return wxT(" ");
    }
}

const wchar_t *Srpski_Cyr::getWXText(int page, int text) {

    switch (page) {
        case GENERAL:
            switch (text) {
                case ERROR_LABEL: return wxT("Грешка");
                    break;
                case NEXT_BUTTON_LABEL: return wxT("Даље >");
                    break;
                case CANCEL_BUTTON_LABEL: return wxT("Откажи");
                    break;
                case BACK_BUTTON_LABEL: return wxT("< Претходна");
                    break;
                case RETRIEVE_BUTTON_LABEL: return wxT("&Од-Обриши");
                    break;
                case BROWSE_BUTTON_LABEL: return wxT("Пронађи");
                    break;
                case FINISH_BUTTON_LABEL: return wxT("Заврши");
                    break;
                case PROGRESS_DIALOG_QUESTION: return wxT("Питање претраживача!");
                    break;
                case FILE_NAME: return wxT("Име фајла");
                    break;
                case PROGRESS_DIALOG_ABORTED: return wxT("Претраживач прекинут!");
                    break;
                default: return wxT(" ");
            }
        case GREETINGS_PAGE:
            switch (text) {
                case WELCOME: return wxT("Добро нам дошли у");
                    break;
                case WINDOWS_UNDELETE_WIZARD: return wxT("ОД-ОБРИШИ ЗА WINDOWS");
                    break;
                case THIS_WIZARD_WILL_GUIDE_YOU: return
                    wxT(" Овај помоћник ће вас провести")
                    wxT(" кроз 3 корака ")
                    wxT(" поступка од-обрисавања\n");
                    break;
                case THE_UNDELETE_PROCESS_CONSISTS: return
                    wxT("Поступак од-обрисавања се састоји од 3 једноставна корака:\n\n")
                    wxT("- Корак 1: Означите диск са кога желите да од-обришете обрисане фајлове.\n\n")
                    wxT("- Корак 2: Означите жељену одредишну фасциклу.\n")
                    wxT("               У њу ће GetMyFilesBack снимити од-обрисане фајлове.\n")
                    wxT("               Ta фасцикла не сме бити на диску наведеном у кораку 1.\n\n")
                    wxT("- Корак 3: Означите фајлове које желите да од-обришете.\n\n\n\n");
                    break;
                case CLICK_NEXT_TO_CONTINUE: return wxT("Кликните на 'Даље' да наставите.\n");
                default: return wxT(" ");
                    break;
            }
            break;
        case SELECT_SOURCE_DRIVE_PAGE:
            switch (text) {
                case PLEASE_SELECT_SOURCE: return
                    wxT("Означите диск на коме сте обрисали фајлове и кликните на 'Даље' ");
                    break;
                case PLEASE_SELECT_ONE_DRIVE: return wxT("Молим селектујте један диск!");
                    break;
                case ERROR_LOADING_FILES: return wxT("Проблем при учитавању фајлова\nПроверита права приступа!");
                    break;
                default: return wxT(" ");
            }
        case SELECT_DESTINATION_PAGE:
            switch (text) {
                case PLEASE_SELECT_DESTINATION: return
                    wxT("Молим означите одредиште ")
                    wxT("за од обрисане фајлове и ")
                    wxT("кликните на  \'Даље\'");
                    break;
                case SAME_DRIVE_ERROR: return
                    wxT("Фасцикла коју сте ознчили се налази на истом диску као и фајлови које желите да ођобришете\n")
                    wxT("Молим одаберите фасциклу на другом диску!");
                    break;
                case CANT_CREATE_THREADS_ERROR: return wxT("Не могу да направим нит за читање диска.");
                    break;
                case DO_YOU_REALLY_WANT_TO_CANCEL: return wxT("Да ли заиста желите да одустанете?");
                    break;

                default: return wxT(" ");
            }
        case SELECT_FILES_PAGE:
            switch (text) {
                case SELECT_ALL_BUTTON_LABEL: return wxT("Означи све");
                    break;
                case UNSELECT_ALL_BUTTON_LABEL: return wxT("Од-означи све");
                    break;
                case ALL_IMAGES_BUTTON_LABEL: return wxT("Све слике");
                    break;
                case ALL_VIDEOS_BUTTON_LABEL: return wxT("Сав видео");
                    break;
                case ALL_MUSIC_BUTTON_LABEL: return wxT("Сву музику");
                    break;
                case SELECT_FILES: return
                    wxT("Означите фајлове које желите да покушате да ођобришете.\n\n")
                    wxT("Да би сте означили више фајлове, држите притиснуто Ctrl\n\n.");
                    break;
                case SELECT_FILES_OTHER: return
                    wxT("Пронашао %d фајлова.\n")
                    wxT("Означите фајлове које желите да покушате да ођобришете и кликните на \'Од-Обриши\'\n\n")
                    wxT("Да би сте означили више фајлова, стисните и држите тестер Ctrl.");
                    break;
                default: return wxT(" ");
            }
        case END_PAGE:
            switch (text) {
                case SUCCESSFULLY_RETRIEVED: return
                    wxT("Успешно од-обрисано %d фајлова од %d означена\n\n\n")
                    wxT("Можете их пронаћи у\n%s.");
            }
        default: return wxT(" ");
    }
}

const wchar_t *Deutsch::getWXText(int page, int text) {

    switch (page) {
        case GENERAL:
            switch (text) {
                case ERROR_LABEL: return wxT("Fehler");
                    break;
                case NEXT_BUTTON_LABEL: return wxT("Weiter >");
                    break;
                case CANCEL_BUTTON_LABEL: return wxT("Abbrechen");
                    break;
                case BACK_BUTTON_LABEL: return wxT("< Zurück");
                    break;
                case RETRIEVE_BUTTON_LABEL: return wxT("&Retten >");
                    break;
                case BROWSE_BUTTON_LABEL: return wxT("Blättern");
                    break;
                case FINISH_BUTTON_LABEL: return wxT("Beenden");
                    break;
                case PROGRESS_DIALOG_QUESTION: return wxT("Fortschrittsdialogfrage");
                    break;
                case FILE_NAME: return wxT("Dateinamen");
                    break;
                case PROGRESS_DIALOG_ABORTED: return wxT("Dialog Fortschritt abgebrochen!");
                    break;
                default: return wxT(" ");
            }
        case GREETINGS_PAGE:
            switch (text) {
                case WELCOME: return wxT("Willkommen im");
                    break;
                case WINDOWS_UNDELETE_WIZARD: return wxT("Windows Undelete Assistent");
                    break;
                case THIS_WIZARD_WILL_GUIDE_YOU: return
                    wxT("Dieser Assistent führt Sie  ")
                    wxT("durch die 3-Schritt- ")
                    wxT("Undelete-Prozess\n");
                    break;
                case THE_UNDELETE_PROCESS_CONSISTS: return
                    wxT("Die Undelete-Prozess besteht aus drei einfachen Schritten:\n\n")
                    wxT("- Schritt 1: Wählen Sie das Quelllaufwerk, von dem Sie gelöschte Dateien\n                wiederherstellen möchten.\n\n")
                    wxT("- Schritt 2: Wählen Sie den Zielordner.\n")
                    wxT("               Dies ist, wo GetMyFilesBack wird wiederhergestellten Dateien\n              speichern.\n")
                    wxT("               Dieser Ordner muss nicht auf dem Quelllaufwerk sein.\n\n")
                    wxT("- Schritt 3: Wählen Sie die Dateien, die Sie wiederherstellen möchten.\n\n\n\n");
                    break;
                case CLICK_NEXT_TO_CONTINUE: return wxT("Klicken Sie auf 'Weiter'\n");
                default: return wxT(" ");
                    break;
            }
            break;
        case SELECT_SOURCE_DRIVE_PAGE:
            switch (text) {
                case PLEASE_SELECT_SOURCE: return
                    wxT("Bitte wählen Sie das Quelllaufwerk, ")
                    wxT("in dem Sie Dateien gelöscht haben ")
                    wxT("und klicken Sie auf \'Weiter\'\n");
                    break;
                case PLEASE_SELECT_ONE_DRIVE: return wxT("Bitte wählen Sie ein Laufwerk!");
                    break;
                case ERROR_LOADING_FILES: return wxT("Fehler beim Laden von Dateien\nÜberprüfen Sie Zugriffsrechte!");
                    break;
                default: return wxT(" ");
            }
        case SELECT_DESTINATION_PAGE:
            switch (text) {
                case PLEASE_SELECT_DESTINATION: return
                    wxT("Bitte wählen Sie das Ziel ")
                    wxT("für die abgerufenen Dateien ")
                    wxT("und klicken Sie auf \'Weiter\'");
                    break;
                case SAME_DRIVE_ERROR: return
                    wxT("Der Ordner, den Sie ausgewählt haben, ist auf dem gleichen Laufwerk wie die Dateien, die abgerufen werden müssen!\n")
                    wxT("Bitte geben Sie einen Ordner auf einem anderen Laufwerk!");
                    break;
                case CANT_CREATE_THREADS_ERROR: return wxT("Den Thread zum Lesen des Laufwerk kann nicht erstellt werden.");
                    break;
                case DO_YOU_REALLY_WANT_TO_CANCEL: return wxT("Wollen Sie wirklich abbrechen?");
                    break;

                default: return wxT(" ");
            }
        case SELECT_FILES_PAGE:
            switch (text) {
                case SELECT_ALL_BUTTON_LABEL: return wxT("Alle auswählen");
                    break;
                case UNSELECT_ALL_BUTTON_LABEL: return wxT("Alle abwählen");
                    break;
                case ALL_IMAGES_BUTTON_LABEL: return wxT("Alle Bilder");
                    break;
                case ALL_VIDEOS_BUTTON_LABEL: return wxT("Alle Videos");
                    break;
                case ALL_MUSIC_BUTTON_LABEL: return wxT("Alle Musik");
                    break;
                case SELECT_FILES: return
                    wxT("Wählen Sie die Dateien, die Sie versuchen, abrufen möchten.\n\n")
                    wxT("Um mehrere Dateien auszuwählen, halten Sie Strg\n\n.");
                    break;
                case SELECT_FILES_OTHER: return
                    wxT("%d Dateien gefunden.\n")
                    wxT("Wählen Sie die Dateien abrufen, um zu versuchen, und klicken Sie auf \'Retten\'\n\n")
                    wxT("Um mehrere Dateien auszuwählen, halten Sie Strg.");
                    break;
                default: return wxT(" ");
            }
        case END_PAGE:
            switch (text) {
                case SUCCESSFULLY_RETRIEVED: return
                    wxT("Erfolgreich abgerufen %d Dateien von %d ausgewählt\n\n\n")
                    wxT("Sie können sie hier finden:\n%s.");
            }
        default: return wxT(" ");
    }
}
