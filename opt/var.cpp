/*@
XOC Release License

Copyright (c) 2013-2014, Alibaba Group, All rights reserved.

    compiler@aliexpress.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Su Zhenyu nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

author: Su Zhenyu
@*/
#include "cominc.h"

namespace xoc {

VarMgr::VarMgr(RegionMgr * rm)
{
    ASSERT0(rm);
    m_var_count = 1; //for enjoying bitset util
    m_str_count = 1;
    m_ru_mgr = rm;
    m_tm = rm->get_type_mgr();
}


//
//START VAR
//
VAR::VAR()
{
    VAR_id(this) = 0; //unique id;
    VAR_type(this) = UNDEF_TYID;
    VAR_name(this) = NULL;
    VAR_str(this) = 0;
    u2.flag = 0; //Record various properties of variable.
    align = 0;
}


void VAR::dump(FILE * h, TypeMgr const* dm) const
{
    CHAR buf[MAX_BUF_LEN];
    buf[0] = 0;
    if (h == NULL) {
        h = g_tfile;
    }
    if (h == NULL) { return; }
    fprintf(h, "\n");
    dumpIndent(h, g_indent);
    fprintf(h, "%s", dump(buf, dm));
    fflush(h);
}


//You must make sure this function will not change any field of VAR.
CHAR * VAR::dump(CHAR * buf, TypeMgr const* dm) const
{
    CHAR * tb = buf;
    CHAR * name = SYM_name(VAR_name(this));
    CHAR tt[43];
    if (xstrlen(name) > 43) {
        memcpy(tt, name, 43);
        tt[39] = '.';
        tt[40] = '.';
        tt[41] = '.';
        tt[42] = 0;
        name = tt;
    }
    buf += sprintf(buf, "VAR%d(%s):", VAR_id(this), name);
    if (HAVE_FLAG(VAR_flag(this), VAR_GLOBAL)) {
        strcat(buf, "global");
    } else if (HAVE_FLAG(VAR_flag(this), VAR_LOCAL)) {
        strcat(buf, "local");
    } else {
        UNREACH();
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_STATIC)) {
        strcat(buf, ",static");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_READONLY)) {
        strcat(buf, ",const");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_VOLATILE)) {
        strcat(buf, ",volatile");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_RESTRICT)) {
        strcat(buf, ",restrict");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_HAS_INIT_VAL)) {
        strcat(buf, ",has_init_val");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_FUNC_DECL)) {
        strcat(buf, ",func_decl");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_FAKE)) {
        strcat(buf, ",fake");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_LABEL)) {
        strcat(buf, ",label");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_FORMAL_PARAM)) {
        strcat(buf, ",formal_param");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_SPILL)) {
        strcat(buf, ",spill_loc");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_ADDR_TAKEN)) {
        strcat(buf, ",addr_taken");
    }

    if (is_string()) {
        strcat(buf, ",str");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_ARRAY)) {
        strcat(buf, ",array");
    }

    if (HAVE_FLAG(VAR_flag(this), VAR_IS_ALLOCABLE)) {
        strcat(buf, ",allocable");
    }

    buf += strlen(buf);

    Type const* type = VAR_type(this);
    ASSERT0(type);

    if (is_pointer()) {
        buf += strlen(buf);
        sprintf(buf, ",pointer,pt_base_sz:%d", TY_ptr_base_size(type));
    }

    sprintf(buf, ",%s", dm->get_dtype_name(TY_dtype(type)));
    if (TY_dtype(type) > D_F128) {
        buf += strlen(buf);
        sprintf(buf, ",mem_size:%d", getByteSize(dm));
    }

    if (VAR_align(this) != 0) {
        buf += strlen(buf);
        sprintf(buf, ",align:%d", VAR_align(this));
    }

    strcat(buf, ",decl:'");
    buf += strlen(buf);
    dumpVARDecl(buf, 40);
    strcat(buf, "'");

    #ifdef _DEBUG_
    UINT tmpf = VAR_flag(this);
    REMOVE_FLAG(tmpf, VAR_GLOBAL);
    REMOVE_FLAG(tmpf, VAR_LOCAL);
    REMOVE_FLAG(tmpf, VAR_STATIC);
    REMOVE_FLAG(tmpf, VAR_READONLY);
    REMOVE_FLAG(tmpf, VAR_VOLATILE);
    REMOVE_FLAG(tmpf, VAR_HAS_INIT_VAL);
    REMOVE_FLAG(tmpf, VAR_FUNC_DECL);
    REMOVE_FLAG(tmpf, VAR_FAKE);
    REMOVE_FLAG(tmpf, VAR_IS_LABEL);
    REMOVE_FLAG(tmpf, VAR_IS_ARRAY);
    REMOVE_FLAG(tmpf, VAR_IS_FORMAL_PARAM);
    REMOVE_FLAG(tmpf, VAR_IS_SPILL);
    REMOVE_FLAG(tmpf, VAR_ADDR_TAKEN);
    REMOVE_FLAG(tmpf, VAR_IS_PR);
    REMOVE_FLAG(tmpf, VAR_IS_RESTRICT);
    REMOVE_FLAG(tmpf, VAR_IS_ALLOCABLE);
    ASSERT0(tmpf == 0);
    #endif
    return tb;
}
//END VAR


//
//START VarMgr
//
//Free VAR memory.
void VarMgr::destroyVar(VAR * v)
{
    ASSERT0(VAR_id(v) != 0);
    m_freelist_of_varid.bunion(VAR_id(v), *m_ru_mgr->get_sbs_mgr());
    m_var_vec.set(VAR_id(v), NULL);
    delete v;
}


void VarMgr::destroy()
{
    for (INT i = 0; i <= m_var_vec.get_last_idx(); i++) {
        VAR * v = m_var_vec.get((UINT)i);
        if (v == NULL) { continue; }
        delete v;
    }

    m_freelist_of_varid.clean(*m_ru_mgr->get_sbs_mgr());
}


void VarMgr::assignVarId(VAR * v)
{
    SEGIter * iter = NULL;
    INT id = m_freelist_of_varid.get_first(&iter);
    ASSERT0(id != 0);
    if (id > 0) {
        m_freelist_of_varid.diff(id, *m_ru_mgr->get_sbs_mgr());
        VAR_id(v) = id;
    } else {
        VAR_id(v) = (UINT)m_var_count++;
    }
    ASSERT(VAR_id(v) < 5000000, ("too many variables"));
    ASSERT0(m_var_vec.get(VAR_id(v)) == NULL);
    m_var_vec.set(VAR_id(v), v);
}


//Add VAR into VarTab.
//Note you should call this function cafefully, and make sure
//the VAR is unique. This function does not keep the uniqueness
//related to properties.
//'var_name': name of the variable, it is optional.
VAR * VarMgr::registerVar(
        CHAR const* varname,
        Type const* type,
        UINT align,
        UINT flag)
{
    ASSERT0(varname);
    SYM * sym = m_ru_mgr->addToSymbolTab(varname);
    return registerVar(sym, type, align, flag);
}


//Add VAR into VarTab.
//Note you should call this function cafefully, and make sure
//the VAR is unique. This function does not keep the uniqueness
//related to properties.
//'var_name': name of the variable, it is optional.
VAR * VarMgr::registerVar(
        SYM const* var_name,
        Type const* type,
        UINT align,
        UINT flag)
{
    ASSERT0(type);
    ASSERT(var_name, ("variable need a name"));

    //VAR is string type, but not const string.
    //ASSERT(!type->is_string(), ("use registerStringVar instead of"));

    VAR * v = allocVAR();
    VAR_type(v) = type;
    VAR_name(v) = var_name;
    VAR_align(v) = align;
    VAR_flag(v) = flag;
    assignVarId(v);
    return v;
}


//Register VAR for const string.
//Return VAR if there is already related to 's',
//otherwise create a new VAR.
//'var_name': name of the variable, it is optional.
//'s': string's content.
VAR * VarMgr::registerStringVar(CHAR const* var_name, SYM const* s, UINT align)
{
    ASSERT0(s != NULL);
    VAR * v;
    if ((v = m_str_tab.get(s)) != NULL) {
        return v;
    }

    v = allocVAR();

    CHAR buf[64];
    if (var_name == NULL) {
        sprintf(buf, ".rodata_%lu", (ULONG)m_str_count++);
        VAR_name(v) = m_ru_mgr->addToSymbolTab(buf);
    } else {
        VAR_name(v) = m_ru_mgr->addToSymbolTab(var_name);
    }

    VAR_str(v) = s;
    VAR_type(v) = m_tm->getString();
    VAR_align(v) = align;
    VAR_is_global(v) = true; //store in .data or .rodata
    VAR_allocable(v) = true;
    assignVarId(v);
    m_str_tab.set(s, v);
    return v;
}


//Dump all variables registered.
void VarMgr::dump(CHAR * name)
{
    FILE * h = g_tfile;
    if (name != NULL) {
        h = fopen(name, "a+");
        ASSERT0(h);
    }
    if (h == NULL) { return; }

    fprintf(h, "\n\nVAR to Decl Mapping:");

    CHAR buf[4096]; //WORKAROUND, should allocate buffer adaptive.
    for (INT i = 0; i <= m_var_vec.get_last_idx(); i++) {
        VAR * v = m_var_vec.get(i);
        if (v == NULL) { continue; }
        buf[0] = 0;
        fprintf(h, "\n%s", v->dump(buf, m_tm));
        fflush(h);
    }

    fprintf(h, "\n");
    fflush(h);
    if (h != g_tfile) {
        fclose(h);
    }
}
//END VarMgr

} //namespace xoc
