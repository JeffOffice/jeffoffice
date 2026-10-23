#include "calc/numfmtnames.h"

#include <QStringList>

namespace jp::calc {

namespace {

struct Names {
    QString months[2][12];   // full, abbreviated
    QString days[2][7];
};

QStringList split(const char *utf8) { return QString::fromUtf8(utf8).split(QLatin1Char(',')); }

Names make(const char *months, const char *monthsShort, const char *days, const char *daysShort)
{
    Names n;
    const QStringList lists[4] = {split(months), split(monthsShort), split(days), split(daysShort)};
    for (int i = 0; i < 12; ++i) {
        n.months[0][i] = lists[0].value(i);
        n.months[1][i] = lists[1].value(i);
    }
    for (int i = 0; i < 7; ++i) {
        n.days[0][i] = lists[2].value(i);
        n.days[1][i] = lists[3].value(i);
    }
    return n;
}

const Names &names(int lang)
{
    static const Names all[7] = {
        make("January,February,March,April,May,June,July,August,September,October,November,December", "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec",
             "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday", "Sun,Mon,Tue,Wed,Thu,Fri,Sat"),
        make("Januar,Februar,März,April,Mai,Juni,Juli,August,September,Oktober,November,Dezember", "Jan,Feb,Mrz,Apr,Mai,Jun,Jul,Aug,Sep,Okt,Nov,Dez",
             "Sonntag,Montag,Dienstag,Mittwoch,Donnerstag,Freitag,Samstag", "So,Mo,Di,Mi,Do,Fr,Sa"),
        make("janvier,février,mars,avril,mai,juin,juillet,août,septembre,octobre,novembre,décembre", "janv.,févr.,mars,avr.,mai,juin,juil.,août,sept.,oct.,nov.,déc.",
             "dimanche,lundi,mardi,mercredi,jeudi,vendredi,samedi", "dim.,lun.,mar.,mer.,jeu.,ven.,sam."),
        make("enero,febrero,marzo,abril,mayo,junio,julio,agosto,septiembre,octubre,noviembre,diciembre", "ene,feb,mar,abr,may,jun,jul,ago,sep,oct,nov,dic",
             "domingo,lunes,martes,miércoles,jueves,viernes,sábado", "dom,lun,mar,mié,jue,vie,sáb"),
        make("gennaio,febbraio,marzo,aprile,maggio,giugno,luglio,agosto,settembre,ottobre,novembre,dicembre", "gen,feb,mar,apr,mag,giu,lug,ago,set,ott,nov,dic",
             "domenica,lunedì,martedì,mercoledì,giovedì,venerdì,sabato", "dom,lun,mar,mer,gio,ven,sab"),
        make("janeiro,fevereiro,março,abril,maio,junho,julho,agosto,setembro,outubro,novembro,dezembro", "jan,fev,mar,abr,mai,jun,jul,ago,set,out,nov,dez",
             "domingo,segunda-feira,terça-feira,quarta-feira,quinta-feira,sexta-feira,sábado", "dom,seg,ter,qua,qui,sex,sáb"),
        make("januari,februari,maart,april,mei,juni,juli,augustus,september,oktober,november,december", "jan,feb,mrt,apr,mei,jun,jul,aug,sep,okt,nov,dec",
             "zondag,maandag,dinsdag,woensdag,donderdag,vrijdag,zaterdag", "zo,ma,di,wo,do,vr,za"),
    };
    return all[lang >= 0 && lang < 7 ? lang : 0];
}

const QString &empty()
{
    static const QString e;
    return e;
}

} // namespace

int formatLanguageOf(quint32 lcid)
{
    switch (lcid & 0x3FF) {
    case 0x07: return 1;
    case 0x0C: return 2;
    case 0x0A: return 3;
    case 0x10: return 4;
    case 0x16: return 5;
    case 0x13: return 6;
    default: return 0;
    }
}

int formatLanguageOfTag(const QString &primary)
{
    static const char *const tags[] = {"en", "de", "fr", "es", "it", "pt", "nl"};
    for (int i = 0; i < 7; ++i)
        if (primary == QLatin1String(tags[i])) return i;
    return 0;
}

const QString &formatMonthName(int lang, int month, int form)
{
    if (month < 1 || month > 12) return empty();
    return names(lang).months[form ? 1 : 0][month - 1];
}

const QString &formatDayName(int lang, int weekday, int form)
{
    if (weekday < 0 || weekday > 6) return empty();
    return names(lang).days[form ? 1 : 0][weekday];
}

} // namespace jp::calc
