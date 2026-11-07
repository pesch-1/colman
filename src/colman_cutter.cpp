// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Functions to compute a Cutter number for a book.
/**
   @file
   @author Christoph Pesch
 */

#include "colman_cutter.hpp"

#ifndef SYSTEM_CCTYPE
#  include <cctype>
#  define SYSTEM_CCTYPE
#endif
#ifndef SYSTEM_CSTRING
#  include <cstring>
#  define SYSTEM_CSTRING
#endif

namespace
{

  /// One entry in the Cutter-Sanborn Three-Figure Table.
  struct CutterEntry
  {
    char const* prefix; ///< lowercase word beginning (includes first letter)
    int number;         ///< Cutter number, 1–999 (monotone within first letter)
  };

  /// Cutter-Sanborn Three-Figure Table, adapted from the standard edition.
  /**
     Within each first-letter section the numbers are strictly monotone with
     alphabetical order.  Numbers are in the range 1–999.
     Entries without an explicit row comment belong to the nearest preceding
     letter comment.
   */
  CutterEntry const CutterTable[] = {
    // -- A --
      {"aa", 110},
      {"ab", 120},
      {"ac", 130},
      {"ad", 140},
      {"ae", 150},
      {"af", 160},
      {"ag", 170},
      {"ah", 180},
      {"ai", 190},
      {"aj", 191},
      {"ak", 192},
      {"al", 200},
      {"ald", 214},
      {"ale", 215},
      {"ali", 216},
      {"all", 217},
      {"alm", 218},
      {"alo", 220},
      {"alp", 230},
      {"als", 250},
      {"alt", 260},
      {"alv", 280},
      {"alw", 290},
      {"am", 300},
      {"amb", 310},
      {"ame", 320},
      {"ami", 330},
      {"amo", 340},
      {"amp", 350},
      {"an", 400},
      {"and", 410},
      {"ang", 430},
      {"ann", 470},
      {"ans", 490},
      {"ant", 495},
      {"ap", 500},
      {"ar", 600},
      {"arc", 610},
      {"arg", 640},
      {"arm", 670},
      {"art", 690},
      {"as", 700},
      {"ash", 720},
      {"at", 800},
      {"ath", 810},
      {"au", 900},
      {"aud", 910},
      {"aug", 920},
      {"aus", 960},
      {"av", 973},
      {"aw", 976},
      {"az", 980},
    // -- B --
      {"ba", 110},
      {"bab", 120},
      {"bac", 130},
      {"bad", 140},
      {"bai", 190},
      {"bak", 170},
      {"bal", 200},
      {"ban", 220},
      {"bar", 230},
      {"bas", 240},
      {"bat", 250},
      {"bau", 260},
      {"be", 300},
      {"bec", 310},
      {"bed", 320},
      {"beh", 340},
      {"bel", 370},
      {"ben", 390},
      {"ber", 400},
      {"bes", 410},
      {"bet", 420},
      {"bi", 500},
      {"bil", 510},
      {"bio", 530},
      {"bl", 600},
      {"bla", 610},
      {"ble", 620},
      {"blo", 640},
      {"blu", 650},
      {"bo", 700},
      {"bol", 710},
      {"bon", 720},
      {"boo", 730},
      {"bor", 740},
      {"bos", 750},
      {"br", 800},
      {"bra", 810},
      {"bre", 820},
      {"bri", 830},
      {"bro", 840},
      {"bru", 850},
      {"bu", 900},
      {"bul", 910},
      {"bun", 920},
      {"bur", 930},
      {"by", 960},
    // -- C --
      {"ca", 110},
      {"cal", 200},
      {"cam", 210},
      {"can", 220},
      {"car", 230},
      {"cas", 240},
      {"cat", 250},
      {"cau", 260},
      {"ce", 300},
      {"ch", 400},
      {"cha", 410},
      {"che", 420},
      {"chi", 430},
      {"cho", 440},
      {"chu", 450},
      {"ci", 500},
      {"cl", 600},
      {"cla", 610},
      {"cli", 630},
      {"clo", 640},
      {"co", 700},
      {"col", 710},
      {"com", 720},
      {"con", 730},
      {"cop", 740},
      {"cor", 750},
      {"cou", 760},
      {"cr", 800},
      {"cra", 810},
      {"cre", 820},
      {"cri", 830},
      {"cro", 840},
      {"cu", 900},
      {"cur", 930},
      {"cut", 940},
    // -- D --
      {"da", 110},
      {"dal", 200},
      {"dam", 210},
      {"dan", 220},
      {"dar", 230},
      {"das", 240},
      {"dau", 260},
      {"de", 300},
      {"ded", 310},
      {"del", 370},
      {"dem", 380},
      {"den", 390},
      {"der", 400},
      {"des", 410},
      {"dev", 440},
      {"di", 500},
      {"dic", 510},
      {"dif", 520},
      {"dig", 530},
      {"din", 550},
      {"dis", 560},
      {"dit", 570},
      {"do", 600},
      {"doc", 620},
      {"dom", 670},
      {"don", 680},
      {"dor", 700},
      {"dos", 710},
      {"dou", 720},
      {"dr", 800},
      {"dra", 810},
      {"dri", 830},
      {"dro", 840},
      {"du", 900},
      {"dum", 920},
      {"dun", 930},
    // -- E --
      {"ea", 110},
      {"ear", 130},
      {"eas", 140},
      {"ed", 200},
      {"ede", 210},
      {"edg", 220},
      {"edi", 230},
      {"edu", 260},
      {"ef", 400},
      {"eg", 500},
      {"ei", 600},
      {"eis", 620},
      {"el", 800},
      {"ela", 810},
      {"ele", 820},
      {"eli", 830},
      {"ell", 850},
      {"els", 860},
      {"em", 900},
      {"eme", 910},
      {"en", 950},
      {"eng", 960},
      {"ep", 970},
      {"er", 980},
    // -- F --
      {"fa", 110},
      {"fac", 120},
      {"fal", 150},
      {"fam", 160},
      {"fan", 170},
      {"far", 180},
      {"fat", 190},
      {"fe", 200},
      {"fed", 210},
      {"fee", 220},
      {"fen", 230},
      {"fer", 240},
      {"fi", 300},
      {"fic", 310},
      {"fie", 320},
      {"fig", 330},
      {"fil", 340},
      {"fin", 350},
      {"fis", 360},
      {"fit", 370},
      {"fl", 400},
      {"fla", 410},
      {"fle", 420},
      {"flo", 440},
      {"flu", 450},
      {"fo", 500},
      {"fol", 510},
      {"fon", 520},
      {"for", 530},
      {"fos", 540},
      {"fr", 600},
      {"fra", 610},
      {"fre", 620},
      {"fri", 630},
      {"fro", 640},
      {"fu", 700},
      {"ful", 710},
      {"fun", 720},
      {"fur", 730},
    // -- G --
      {"ga", 110},
      {"gal", 150},
      {"gam", 160},
      {"gan", 170},
      {"gar", 180},
      {"gas", 190},
      {"ge", 200},
      {"gen", 220},
      {"geo", 230},
      {"ger", 240},
      {"gi", 300},
      {"gil", 310},
      {"gin", 320},
      {"gl", 400},
      {"gla", 410},
      {"gle", 420},
      {"glo", 440},
      {"go", 500},
      {"goe", 510},
      {"gol", 530},
      {"gon", 540},
      {"goo", 550},
      {"gor", 560},
      {"gos", 570},
      {"gr", 600},
      {"gra", 610},
      {"gre", 620},
      {"gri", 630},
      {"gro", 640},
      {"gu", 700},
      {"gua", 710},
      {"gue", 720},
      {"gui", 730},
      {"gun", 760},
      {"gy", 800},
    // -- H --
      {"ha", 110},
      {"hab", 120},
      {"hac", 130},
      {"had", 140},
      {"hae", 150},
      {"haf", 160},
      {"hag", 170},
      {"hah", 180},
      {"hai", 190},
      {"hak", 192},
      {"hal", 200},
      {"ham", 210},
      {"han", 220},
      {"har", 230},
      {"has", 240},
      {"hat", 250},
      {"hau", 260},
      {"he", 300},
      {"hed", 340},
      {"hee", 350},
      {"hei", 360},
      {"hel", 370},
      {"hem", 380},
      {"hemi", 475},
      {"hen", 490},
      {"her", 500},
      {"hes", 510},
      {"het", 520},
      {"hi", 600},
      {"hig", 610},
      {"hil", 620},
      {"hin", 630},
      {"his", 640},
      {"ho", 700},
      {"hob", 710},
      {"hoc", 720},
      {"hoe", 730},
      {"hof", 740},
      {"hog", 750},
      {"hol", 760},
      {"hom", 770},
      {"hon", 780},
      {"hoo", 790},
      {"hor", 800},
      {"hos", 810},
      {"hot", 820},
      {"hou", 830},
      {"hu", 900},
      {"hub", 910},
      {"hug", 920},
      {"hul", 930},
      {"hum", 940},
      {"hun", 950},
      {"hur", 960},
      {"hus", 970},
      {"hut", 980},
      {"hy", 990},
    // -- I --
      {"ia", 110},
      {"ic", 120},
      {"id", 130},
      {"ie", 140},
      {"ig", 150},
      {"il", 170},
      {"im", 200},
      {"in", 300},
      {"ind", 310},
      {"inf", 320},
      {"ing", 330},
      {"ini", 340},
      {"inn", 350},
      {"ins", 370},
      {"int", 380},
      {"inv", 390},
      {"io", 400},
      {"ir", 500},
      {"ire", 510},
      {"irl", 520},
      {"is", 600},
      {"isc", 610},
      {"ism", 620},
      {"it", 700},
      {"ite", 710},
      {"its", 720},
    // -- J --
      {"ja", 110},
      {"jac", 120},
      {"jad", 130},
      {"jan", 160},
      {"jar", 170},
      {"je", 200},
      {"jea", 210},
      {"jen", 230},
      {"jer", 240},
      {"jes", 250},
      {"ji", 300},
      {"jo", 400},
      {"joc", 410},
      {"joe", 420},
      {"joh", 430},
      {"jon", 440},
      {"jos", 470},
      {"ju", 500},
      {"jud", 510},
      {"jun", 530},
      {"jur", 540},
    // -- K --
      {"ka", 110},
      {"kal", 150},
      {"kam", 160},
      {"kan", 170},
      {"kar", 180},
      {"ke", 200},
      {"kel", 240},
      {"ken", 250},
      {"ker", 260},
      {"ki", 300},
      {"kil", 310},
      {"kim", 320},
      {"kin", 330},
      {"kip", 340},
      {"kir", 350},
      {"kit", 360},
      {"kl", 400},
      {"kn", 500},
      {"kna", 510},
      {"kno", 520},
      {"ko", 600},
      {"koe", 610},
      {"kom", 620},
      {"kon", 630},
      {"kor", 650},
      {"kr", 700},
      {"ku", 800},
    // -- L --
      {"la", 110},
      {"lab", 120},
      {"lac", 130},
      {"lad", 140},
      {"lag", 150},
      {"lai", 160},
      {"lam", 170},
      {"lan", 180},
      {"lar", 190},
      {"las", 200},
      {"lat", 210},
      {"lau", 220},
      {"lav", 230},
      {"law", 240},
      {"le", 300},
      {"lea", 310},
      {"led", 330},
      {"lee", 340},
      {"lef", 350},
      {"lei", 360},
      {"lem", 370},
      {"len", 380},
      {"leo", 390},
      {"les", 400},
      {"let", 410},
      {"lev", 420},
      {"li", 500},
      {"lib", 510},
      {"lic", 520},
      {"lie", 530},
      {"lig", 540},
      {"lil", 550},
      {"lim", 560},
      {"lin", 570},
      {"lis", 590},
      {"lit", 600},
      {"lo", 700},
      {"loc", 710},
      {"log", 730},
      {"lon", 740},
      {"loo", 750},
      {"lor", 760},
      {"los", 770},
      {"lot", 780},
      {"lou", 790},
      {"lu", 800},
      {"lud", 810},
      {"lum", 830},
      {"lun", 840},
      {"ly", 900},
    // -- M --
      {"ma", 110},
      {"mac", 120},
      {"mad", 130},
      {"mag", 150},
      {"mah", 160},
      {"mai", 170},
      {"mak", 180},
      {"mal", 190},
      {"mam", 200},
      {"man", 210},
      {"mar", 220},
      {"mas", 230},
      {"mat", 240},
      {"mau", 250},
      {"may", 260},
      {"mc", 270},
      {"me", 300},
      {"med", 310},
      {"mei", 320},
      {"mel", 330},
      {"men", 340},
      {"mer", 350},
      {"met", 360},
      {"mi", 400},
      {"mic", 410},
      {"mid", 420},
      {"mil", 430},
      {"min", 440},
      {"mir", 450},
      {"mis", 460},
      {"mo", 500},
      {"mob", 510},
      {"moe", 520},
      {"mol", 530},
      {"mon", 540},
      {"moo", 550},
      {"mor", 560},
      {"mos", 570},
      {"mot", 580},
      {"mou", 590},
      {"mu", 600},
      {"mud", 610},
      {"mun", 620},
      {"mur", 630},
      {"mus", 640},
      {"my", 650},
    // -- N --
      {"na", 110},
      {"nac", 120},
      {"nag", 130},
      {"nan", 140},
      {"nas", 150},
      {"nat", 160},
      {"ne", 200},
      {"ned", 210},
      {"nei", 230},
      {"nel", 240},
      {"ner", 250},
      {"nes", 260},
      {"net", 270},
      {"ni", 300},
      {"nic", 310},
      {"nil", 320},
      {"nit", 330},
      {"no", 400},
      {"nob", 410},
      {"nor", 430},
      {"not", 440},
      {"nov", 450},
      {"nu", 500},
      {"nuc", 510},
      {"num", 520},
      {"ny", 600},
    // -- O --
      {"oa", 110},
      {"ob", 120},
      {"oc", 130},
      {"od", 140},
      {"of", 150},
      {"og", 160},
      {"oh", 170},
      {"oi", 180},
      {"ok", 190},
      {"ol", 200},
      {"ole", 210},
      {"oli", 220},
      {"oll", 230},
      {"om", 300},
      {"on", 400},
      {"one", 410},
      {"oo", 500},
      {"op", 600},
      {"or", 700},
      {"ore", 710},
      {"ori", 720},
      {"orp", 730},
      {"ors", 740},
      {"ort", 750},
      {"os", 800},
      {"ost", 810},
      {"ot", 900},
      {"ou", 920},
      {"out", 930},
      {"ov", 960},
      {"ox", 980},
    // -- P --
      {"pa", 110},
      {"pac", 120},
      {"pad", 130},
      {"pan", 150},
      {"par", 160},
      {"pas", 170},
      {"pat", 180},
      {"pau", 190},
      {"pe", 200},
      {"pea", 210},
      {"pec", 220},
      {"ped", 230},
      {"pel", 250},
      {"pen", 260},
      {"per", 270},
      {"pet", 280},
      {"ph", 300},
      {"phi", 310},
      {"pho", 320},
      {"pi", 400},
      {"pic", 410},
      {"pie", 420},
      {"pig", 430},
      {"pin", 440},
      {"pit", 450},
      {"pl", 500},
      {"pla", 510},
      {"ple", 520},
      {"plo", 540},
      {"plu", 550},
      {"po", 600},
      {"pol", 610},
      {"pon", 620},
      {"pop", 630},
      {"por", 640},
      {"pos", 650},
      {"pot", 660},
      {"pr", 700},
      {"pra", 710},
      {"pre", 720},
      {"pri", 730},
      {"pro", 740},
      {"pu", 800},
      {"pub", 810},
      {"pun", 820},
      {"pur", 830},
    // -- Q --
      {"qu", 110},
      {"que", 120},
      {"qui", 130},
    // -- R --
      {"ra", 110},
      {"rad", 120},
      {"rag", 130},
      {"rai", 150},
      {"ral", 160},
      {"ram", 170},
      {"ran", 180},
      {"rar", 190},
      {"ras", 200},
      {"rat", 210},
      {"rau", 220},
      {"re", 300},
      {"rea", 310},
      {"reb", 320},
      {"rec", 330},
      {"red", 340},
      {"ref", 350},
      {"rel", 370},
      {"rem", 380},
      {"ren", 390},
      {"rep", 400},
      {"res", 410},
      {"ret", 420},
      {"rev", 430},
      {"ri", 500},
      {"ric", 510},
      {"rie", 520},
      {"rig", 530},
      {"ril", 540},
      {"rim", 550},
      {"rin", 560},
      {"ro", 600},
      {"rob", 610},
      {"roc", 620},
      {"rod", 630},
      {"rog", 640},
      {"rol", 650},
      {"rom", 660},
      {"ron", 670},
      {"roo", 680},
      {"ros", 690},
      {"rot", 695},
      {"rou", 700},
      {"ru", 710},
      {"rub", 720},
      {"ruc", 730},
      {"ruf", 740},
      {"rug", 750},
      {"rum", 760},
      {"run", 770},
      {"rus", 780},
      {"ry", 800},
    // -- S --
      {"sa", 110},
      {"sab", 120},
      {"sac", 130},
      {"sad", 140},
      {"saf", 150},
      {"sag", 160},
      {"sal", 170},
      {"sam", 180},
      {"san", 190},
      {"sar", 210},
      {"sat", 230},
      {"sau", 240},
      {"sav", 250},
      {"sc", 300},
      {"sch", 310},
      {"sci", 320},
      {"sco", 330},
      {"se", 400},
      {"sea", 410},
      {"sed", 420},
      {"see", 430},
      {"sei", 450},
      {"sel", 460},
      {"sem", 470},
      {"sen", 480},
      {"ser", 480},
      {"set", 490},
      {"sh", 500},
      {"sha", 510},
      {"shak", 527},
      {"she", 540},
      {"shi", 560},
      {"sho", 570},
      {"shu", 580},
      {"si", 600},
      {"sig", 610},
      {"sil", 620},
      {"sim", 630},
      {"sin", 640},
      {"sir", 650},
      {"sis", 660},
      {"sit", 670},
      {"sl", 700},
      {"sla", 710},
      {"sle", 720},
      {"sli", 730},
      {"slo", 740},
      {"sn", 760},
      {"sno", 770},
      {"so", 800},
      {"sol", 820},
      {"som", 830},
      {"son", 840},
      {"sor", 850},
      {"sp", 860},
      {"spa", 865},
      {"spe", 870},
      {"spi", 875},
      {"spo", 880},
      {"st", 890},
      {"sta", 891},
      {"ste", 893},
      {"sti", 894},
      {"sto", 895},
      {"str", 896},
      {"stu", 897},
      {"su", 920},
      {"sub", 925},
      {"sum", 930},
      {"sun", 935},
      {"sup", 940},
      {"sur", 950},
      {"sus", 960},
      {"sw", 970},
      {"sy", 990},
    // -- T --
      {"ta", 110},
      {"tab", 120},
      {"tac", 130},
      {"tad", 140},
      {"tag", 150},
      {"tal", 160},
      {"tam", 170},
      {"tan", 180},
      {"tar", 190},
      {"tat", 210},
      {"tau", 220},
      {"te", 300},
      {"tea", 310},
      {"ted", 320},
      {"tel", 370},
      {"ten", 380},
      {"ter", 390},
      {"th", 400},
      {"tha", 410},
      {"the", 420},
      {"thi", 430},
      {"tho", 440},
      {"thu", 450},
      {"ti", 500},
      {"til", 510},
      {"tim", 520},
      {"tin", 530},
      {"tit", 540},
      {"to", 600},
      {"tob", 610},
      {"toc", 620},
      {"toe", 630},
      {"tol", 650},
      {"tom", 660},
      {"too", 670},
      {"tor", 680},
      {"tot", 690},
      {"tou", 700},
      {"tr", 800},
      {"tra", 810},
      {"tre", 820},
      {"tri", 830},
      {"tro", 840},
      {"tru", 850},
      {"tu", 900},
      {"tub", 910},
      {"tur", 930},
      {"tw", 960},
      {"ty", 980},
    // -- U --
      {"ub", 110},
      {"uc", 120},
      {"ud", 130},
      {"ue", 140},
      {"uf", 150},
      {"ug", 160},
      {"uh", 170},
      {"ui", 180},
      {"uk", 190},
      {"ul", 200},
      {"um", 300},
      {"un", 400},
      {"und", 410},
      {"uni", 420},
      {"uns", 440},
      {"up", 500},
      {"ur", 600},
      {"urb", 610},
      {"ure", 620},
      {"uri", 630},
      {"us", 700},
      {"ut", 800},
      {"uy", 900},
    // -- V --
      {"va", 110},
      {"van", 150},
      {"var", 160},
      {"vas", 170},
      {"ve", 200},
      {"vel", 220},
      {"ven", 230},
      {"ver", 240},
      {"vi", 300},
      {"vid", 310},
      {"vie", 320},
      {"vil", 330},
      {"vin", 340},
      {"vis", 350},
      {"vo", 400},
      {"vol", 410},
      {"von", 420},
      {"vor", 430},
      {"vu", 500},
    // -- W --
      {"wa", 110},
      {"wal", 150},
      {"wan", 160},
      {"war", 170},
      {"was", 180},
      {"wat", 190},
      {"we", 200},
      {"web", 210},
      {"wed", 220},
      {"wei", 230},
      {"wel", 240},
      {"wen", 250},
      {"wer", 260},
      {"wes", 270},
      {"wh", 300},
      {"whe", 320},
      {"whi", 330},
      {"who", 340},
      {"wi", 400},
      {"wil", 420},
      {"win", 430},
      {"wir", 440},
      {"wis", 450},
      {"wo", 500},
      {"wol", 530},
      {"won", 540},
      {"woo", 550},
      {"wor", 560},
      {"wr", 600},
      {"wu", 700},
    // -- X --
      {"xe", 110},
      {"xi", 120},
      {"xy", 130},
    // -- Y --
      {"ya", 110},
      {"ye", 200},
      {"yea", 210},
      {"yi", 300},
      {"yo", 400},
      {"you", 410},
      {"yu", 500},
    // -- Z --
      {"za", 110},
      {"ze", 200},
      {"zei", 210},
      {"zen", 220},
      {"zi", 300},
      {"zo", 400},
      {"zu", 500},
  };

  /// Transliterate a single UTF-8 char sequence at position \a i in \a s.
  /**
     Returns the ASCII replacement (possibly multi-char) and advances \a i
     past the consumed bytes.  Unknown bytes are returned as-is.
   */
  std::string transliterate_char(std::string const& s, std::size_t& i)
  {
    unsigned char const c = static_cast<unsigned char>(s[i]);
    // 2-byte UTF-8 sequences (Latin-1 Supplement, U+00C0–U+00FF)
    if(c == 0xC3 && i + 1 < s.size()) {
      unsigned char const c2 = static_cast<unsigned char>(s[i + 1]);
      i += 2;
      switch(c2) {
      case 0xA0:
      case 0xA1:
      case 0xA2:
      case 0xA3:
      case 0xA5:
        return "a";
      case 0xA4:
        return "ae"; // ä
      case 0xA6:
        return "ae"; // æ
      case 0xA7:
        return "c";
      case 0xA8:
      case 0xA9:
      case 0xAA:
      case 0xAB:
        return "e";
      case 0xAC:
      case 0xAD:
      case 0xAE:
      case 0xAF:
        return "i";
      case 0xB0:
        return "o";
      case 0xB1:
        return "n";
      case 0xB2:
      case 0xB3:
      case 0xB4:
      case 0xB5:
      case 0xB8:
        return "o";
      case 0xB6:
        return "oe"; // ö
      case 0xB9:
      case 0xBA:
      case 0xBB:
        return "u";
      case 0xBC:
        return "ue"; // ü
      case 0xBD:
        return "y";
      case 0xBF:
        return "ss"; // ß
      case 0x80:
      case 0x81:
      case 0x82:
      case 0x83:
      case 0x85:
        return "a";
      case 0x84:
        return "ae"; // Ä→ae
      case 0x86:
        return "ae"; // Æ
      case 0x87:
        return "c";
      case 0x88:
      case 0x89:
      case 0x8A:
      case 0x8B:
        return "e";
      case 0x8C:
      case 0x8D:
      case 0x8E:
      case 0x8F:
        return "i";
      case 0x90:
        return "o";
      case 0x91:
        return "n";
      case 0x92:
      case 0x93:
      case 0x94:
      case 0x95:
      case 0x98:
        return "o";
      case 0x96:
        return "oe"; // Ö→oe
      case 0x99:
      case 0x9A:
      case 0x9B:
        return "u";
      case 0x9C:
        return "ue"; // Ü→ue
      case 0x9D:
        return "y";
      default:
        return std::string(1, static_cast<char>(c2));
      }
    }
    ++i;
    return std::string(1, static_cast<char>(c));
  }

  /// Normalize a string: lowercase ASCII, transliterate common diacritics.
  std::string normalize(std::string const& s)
  {
    std::string result;
    result.reserve(s.size());
    std::size_t i = 0;
    while(i < s.size()) {
      std::string frag = transliterate_char(s, i);
      for(char ch : frag) {
        result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
      }
    }
    return result;
  }

  /// Remove leading German/English articles from a normalized (lowercase) word.
  std::string strip_articles(std::string const& s)
  {
    static char const* const Articles[] = {
        "der ",
        "die ",
        "das ",
        "dem ",
        "den ",
        "ein ",
        "eine ",
        "einem ",
        "einen ",
        "einer ",
        "eines ",
        "the ",
        "a ",
        "an ",
        "le ",
        "la ",
        "les ",
        "l'",
        "el ",
        "los ",
        "las ",
        "un ",
        "una ",
    };
    for(char const* art : Articles) {
      std::size_t const len = std::strlen(art);
      if(s.size() > len && s.compare(0, len, art) == 0) {
        return s.substr(len);
      }
    }
    return s;
  }

  /// Look up the Cutter-Sanborn number for a lowercase, normalized word.
  /**
     Returns the letter (uppercase) in \a letter and the number (1–999) as
     the return value.  Returns 1 if the word is empty.
   */
  int sanborn_lookup(std::string const& word, char& letter)
  {
    if(word.empty()) {
      letter = '?';
      return 1;
    }
    letter = static_cast<char>(
        std::toupper(static_cast<unsigned char>(word[0])));

    // Find all entries whose prefix starts with the same first character and
    // is a prefix of word.  Take the longest match.
    int best_number = 1; // fallback: single-letter entry
    std::size_t best_len = 0;

    std::size_t const n = sizeof(CutterTable) / sizeof(CutterTable[0]);
    for(std::size_t idx = 0; idx < n; ++idx) {
      std::string const prefix(CutterTable[idx].prefix);
      if(prefix.empty()) {
        continue;
      }
      if(prefix[0] > word[0]) {
        break;
      } // table sorted by letter; no more matches
      if(prefix[0] != word[0]) {
        continue;
      }
      std::size_t const plen = prefix.size();
      if(plen > word.size()) {
        continue;
      }
      if(word.substr(0, plen) == prefix && plen >= best_len) {
        best_len = plen;
        best_number = CutterTable[idx].number;
      }
    }
    return best_number;
  }

  /// Return the first significant (non-article) lowercase letter of a title.
  /**
     Skips leading whitespace and common articles, then returns the first
     alphabetic character, or '\0' if none found.
   */
  char title_initial(std::string const& title)
  {
    std::string const norm = normalize(title);
    std::string const stripped = strip_articles(norm);
    for(char c : stripped) {
      if(std::isalpha(static_cast<unsigned char>(c))) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      }
    }
    return '\0';
  }

  /// Format the Cutter number as a string: letter + zero-padded digits.
  std::string format_cutter_number(char letter, int number)
  {
    std::string result(1, letter);
    result += std::to_string(number);
    return result;
  }

} // anonymous namespace

/**
   Computes a call number signature for a book using the Cutter-Sanborn
   Three-Figure Table.

   The signature has the following format:
   @code
   [DDC ]CutterNumber[title_initial][.n][ year]
   @endcode
   Example: @c "823.914 H475a 1952"

   The parts are assembled as follows:
   - @b DDC: the Dewey Decimal Classification number, omitted if empty.
   - @b CutterNumber: looked up in the Cutter-Sanborn Three-Figure Table
     using @a author_lastname; if @a author_lastname is empty, the first
     significant word of @a title is used instead (leading articles are
     stripped).  The result is an uppercase letter followed by up to three
     digits (e.g. @c H475).
     - @b title_initial: lowercase first significant letter of @a title
       (leading German and English articles are ignored), appended directly
       after the Cutter number.
       - @b year: appended after a space, omitted if empty.
       - @b .n: a decimal suffix (@c .2, @c .3, …) appended to the base
         (before the year) when the generated signature already appears in
         @a existing_signatures, ensuring uniqueness.

   @param author_lastname  Author's last name (may be empty).
   @param title            Book title (must not be empty).
   @param year             Publication year as a string (may be empty).
   @param ddc              Dewey Decimal Classification number (may be empty).
   @param existing_signatures  Set of already assigned signatures in the
   library; used for collision avoidance.
   @return Unique call number signature.
 */
std::string colman::generate_signature(
    std::string const& author_lastname,
    std::string const& title,
    std::string const& year,
    std::string const& ddc,
    std::set<std::string> const& existing_signatures)
{
  // Determine the word to look up: author lastname, or title if no author.
  std::string const lookup_word = author_lastname.empty()
                                      ? normalize(strip_articles(normalize(title)))
                                      : normalize(author_lastname);

  char letter = '\0';
  int const number = sanborn_lookup(lookup_word, letter);
  std::string const cutter_num = format_cutter_number(letter, number);

  char const tinit = title_initial(title);

  // Build base: "{cutter_num}{title_initial}"
  std::string base = cutter_num;
  if(tinit != '\0') {
    base += tinit;
  }

  // Assemble full call number.
  // Format: "{DDC} {base}[ {year}]"  (DDC omitted if empty)
  auto make_call_number = [&](std::string const& suffix) -> std::string {
    std::string result;
    if(!ddc.empty()) {
      result += ddc;
      result += ' ';
    }
    result += base;
    result += suffix;
    if(!year.empty()) {
      result += ' ';
      result += year;
    }
    return result;
  };

  // Try without collision suffix first.
  std::string candidate = make_call_number("");
  if(existing_signatures.find(candidate) == existing_signatures.end()) {
    return candidate;
  }

  // Append ".2", ".3", … until unique.
  for(int n = 2; n < 1000; ++n) {
    candidate = make_call_number('.' + std::to_string(n));
    if(existing_signatures.find(candidate) == existing_signatures.end()) {
      return candidate;
    }
  }

  // Fallback (should not be reached in practice).
  return make_call_number("");
}

/* EOF */
