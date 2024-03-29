//
// Copyright 2017-2018 Giovanni Campagna <gcampagn@cs.stanford.edu>
//
// Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
