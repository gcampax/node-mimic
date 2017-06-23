//
// Copyright 2017 Giovanni Campagna <gcampagn@cs.stanford.edu>
//
// See COPYING for details

#include <mimic.h>
#include <lang/usenglish.h>
#include <lang/cmu_lex.h>
#if 0 // fails to build
#include <lang/cmu_indic_lang.h>
#endif
#include <lang/cmu_grapheme_lang.h>
#include <lang/cmu_grapheme_lex.h>

namespace node_mimic {

void set_lang_list(void)
{
   mimic_add_lang("eng",usenglish_init,cmu_lex_init);
   mimic_add_lang("usenglish",usenglish_init,cmu_lex_init);
#if 0
   mimic_add_lang("cmu_indic_lang",cmu_indic_lang_init,cmu_indic_lex_init);
#endif
   mimic_add_lang("cmu_grapheme_lang",cmu_grapheme_lang_init,cmu_grapheme_lex_init);
}

}
