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
#ifndef __REGION_H__
#define __REGION_H__

namespace xoc {
class IPA;
class CfsMgr;
class IR_DU_MGR;
class IR_AA;
class IR_EXPR_TAB;

//Make sure IR_ICALL is the largest ir.
#define MAX_OFFSET_AT_FREE_TABLE    (sizeof(CICall) - sizeof(IR))

//Analysis Instrument.
//Record Data structure for IR analysis and transformation.
#define ANA_INS_ru_mgr(a)    ((a)->m_ru_mgr)
#define ANA_INS_pass_mgr(a)  ((a)->m_pass_mgr)
#define ANA_INS_var_tab(a)   ((a)->m_ru_var_tab)
class AnalysisInstrument {
public:
    Region * m_ru;
    UINT m_pr_count; //counter of IR_PR.

    SMemPool * m_pool;
    SMemPool * m_du_pool;

    //Indicate a list of IR.
    IR * m_ir_list;

    List<IR const*> * m_call_list; //record CALL/ICALL in region.
    List<IR const*> * m_return_list; //record RETURN in region.

    RegionMgr * m_ru_mgr; //Region manager.
    PassMgr * m_pass_mgr; //PASS manager.

    //Record vars defined in current region,
    //include the subregion defined variables.
    //All LOCAL vars in the tab will be destroyed during region destruction.
    VarTab m_ru_var_tab;
    IR * m_free_tab[MAX_OFFSET_AT_FREE_TABLE + 1];
    Vector<VAR*> m_prno2var; //map prno to related VAR.
    Vector<IR*> m_ir_vector; //record IR which have allocated.
    BitSetMgr m_bs_mgr;
    DefMiscBitSetMgr m_sbs_mgr;
    MDSetMgr m_mds_mgr;
    MDSetHashAllocator m_mds_hash_allocator;
    MDSetHash m_mds_hash;
    List<DU*> m_free_du_list;
    IRBBMgr m_ir_bb_mgr; //Allocate the basic block.
    BBList m_ir_bb_list; //record a list of basic blocks.

    #ifdef _DEBUG_
    BitSet m_has_been_freed_irs;
    #endif
public:
    explicit AnalysisInstrument(Region * ru);
    COPY_CONSTRUCTOR(AnalysisInstrument);
    ~AnalysisInstrument();

    size_t count_mem();
    bool verify_var(VarMgr * vm, VAR * v);
};


//Region referrence info.
#define REF_INFO_maydef(ri)     ((ri)->may_def_mds)
#define REF_INFO_mayuse(ri)     ((ri)->may_use_mds)
class RefInfo {
public:
    MDSet may_def_mds; //Record the MD set for Region usage
    MDSet may_use_mds; //Record the MD set for Region usage

    size_t count_mem()
    {
        size_t c = sizeof(RefInfo);
        c += may_def_mds.count_mem();
        c += may_use_mds.count_mem();
        return c;
    }
};


//
//START Region
//
//Record unique id in RegionMgr.
#define REGION_id(r)                    ((r)->m_id)
#define REGION_type(r)                  ((r)->m_ru_type)
#define REGION_parent(r)                ((r)->m_parent)

//Record analysis data structure for code region.
#define REGION_analysis_instrument(r)   ((r)->m_u1.m_ana_ins)

//Record the binary data for black box region.
#define REGION_blx_data(r)              ((r)->m_u1.m_blx_data)

//Set to true if Region is expected to be inlined.
#define REGION_is_expect_inline(r)      ((r)->m_u2.s1.is_expect_inline)

//Set to true if Region can be inlined.
#define REGION_is_inlinable(r)          ((r)->m_u2.s1.is_inlinable)

//True value means MustDef, MayDef, MayUse are available.
#define REGION_is_mddu_valid(r)         ((r)->m_u2.s1.is_du_valid)

//Record memory reference for region.
#define REGION_refinfo(r)               ((r)->m_ref_info)

//True if region does not modify any memory and live-in variables which
//include VAR and PR.
//This property is very useful if region is a blackbox.
//And readonly region will alleviate the burden of optimizor.
#define REGION_is_readonly(r)            ((r)->m_u2.s1.is_readonly)

class Region {
friend class RegionMgr;
protected:
    union {
        AnalysisInstrument * m_ana_ins; //Analysis instrument.
        void * m_blx_data; //Black box data.
    } m_u1;

protected:
    void scanCallListImpl(
            OUT UINT & num_inner_region,
            IR * irlst,
            OUT List<IR const*> * call_list,
            OUT List<IR const*> * ret_list,
            bool scan_inner_region);
    bool processIRList(OptCtx & oc);
    bool processBBList(OptCtx & oc);
    void prescan(IR const* ir);
    bool partitionRegion();
    bool performSimplify(OptCtx & oc);
    void HighProcessImpl(OptCtx & oc);

public:
    RefInfo * m_ref_info; //record use/def info if Region has.
    REGION_TYPE m_ru_type; //region type.
    UINT m_id; //region unique id.
    VAR * m_var; //record VAR if RU has.
    Region * m_parent; //record parent region.
    union {
        struct {
            BYTE is_expect_inline:1; //see above macro declaration.
            BYTE is_du_valid:1;  //see above macro declaration.

            //True if region does not modify any live-in variables
            //which include VAR and PR. We say the region is readonly.
            BYTE is_readonly:1;

            //True if region can be inlined. Default is false for
            //conservative purpose.
            BYTE is_inlinable:1;
        } s1;
        BYTE s1b1;
    } m_u2;

public:
    explicit Region(REGION_TYPE rt, RegionMgr * rm) { init(rt, rm); }
    COPY_CONSTRUCTOR(Region);
    virtual ~Region() { destroy(); }

    void * xmalloc(UINT size)
    {
        ASSERT(REGION_analysis_instrument(this)->m_pool != NULL,
               ("pool does not initialized"));
        void * p = smpoolMalloc(size, REGION_analysis_instrument(this)->m_pool);
        ASSERT0(p != NULL);
        memset(p, 0, size);
        return p;
    }

    //Add var which used inside current or inner Region.
    //Once the region destructing, all local vars are deleted.
    void addToVarTab(VAR * v)
    { REGION_analysis_instrument(this)->m_ru_var_tab.append(v); }

    //Add irs into IR list of current region.
    void addToIRList(IR * irs)
    { add_next(&REGION_analysis_instrument(this)->m_ir_list, irs); }

    //The function generates new MD for all operations to PR.
    //It should be called if new PR generated in optimzations.
    inline MD const* allocRefForPR(IR * pr)
    {
        MD const* md = genMDforPR(pr);
        pr->setRefMD(md, this);
        pr->cleanRefMDSet();
        return md;
    }

    //The function generates new MD for given LD.
    //It should be called if new PR generated in optimzations.
    inline MD const* allocRefForLoad(IR * ld)
    {
        MD const* md = genMDforLoad(ld);
        ld->setRefMD(md, this);
        ld->cleanRefMDSet();
        return md;
    }

    //The function generates new MD for given ST.
    //It should be called if new PR generated in optimzations.
    inline MD const* allocRefForStore(IR * st)
    {
        MD const* md = genMDforStore(st);
        st->setRefMD(md, this);
        st->cleanRefMDSet();
        return md;
    }

    inline DU * allocDU()
    {
        DU * du = REGION_analysis_instrument(this)->m_free_du_list.remove_head();
        if (du == NULL) {
            du = (DU*)smpoolMallocConstSize(sizeof(DU),
                            REGION_analysis_instrument(this)->m_du_pool);
            memset(du, 0, sizeof(DU));
        }
        return du;
    }

    IRBB * allocBB() { return get_bb_mgr()->allocBB(); }

    //Allocate an IR.
    IR * allocIR(IR_TYPE irt);

    //Allocate AIContainer.
    inline AIContainer * allocAIContainer()
    {
        AIContainer * ai = (AIContainer*)xmalloc(sizeof(AIContainer));
        ASSERT0(ai);
        ai->init();
        return ai;
    }

    //Allocate PassMgr
    virtual PassMgr * allocPassMgr();

    //Allocate AliasAnalysis.
    virtual IR_AA * allocAliasAnalysis();

    IR * buildContinue();
    IR * buildBreak();
    IR * buildCase(IR * casev_exp, LabelInfo const* case_br_lab);
    IR * buildDoLoop(IR * det, IR * init, IR * step, IR * loop_body);
    IR * buildDoWhile(IR * det, IR * loop_body);
    IR * buildWhileDo(IR * det, IR * loop_body);
    IR * buildIf(IR * det, IR * true_body, IR * false_body);
    IR * buildSwitch(
            IR * vexp,
            IR * case_list,
            IR * body,
            LabelInfo const* default_lab);
    IR * buildPRdedicated(UINT prno, Type const* type);
    IR * buildPR(Type const* type);
    UINT buildPrno(Type const* type);
    IR * buildBranch(bool is_true_br, IR * det, LabelInfo const* lab);
    IR * buildId(IN VAR * var_info);
    IR * buildIlabel();
    IR * buildLabel(LabelInfo const* li);
    IR * buildCvt(IR * exp, Type const* tgt_ty);
    IR * buildGoto(LabelInfo const* li);
    IR * buildIgoto(IR * vexp, IR * case_list);
    IR * buildPointerOp(IR_TYPE irt, IR * lchild, IR * rchild);
    IR * buildCmp(IR_TYPE irt, IR * lchild, IR * rchild);
    IR * buildJudge(IR * exp);
    IR * buildBinaryOpSimp(
            IR_TYPE irt,
            Type const* type,
            IR * lchild,
            IR * rchild);
    IR * buildBinaryOp(
            IR_TYPE irt,
            Type const* type,
            IN IR * lchild,
            IN IR * rchild);
    IR * buildUnaryOp(IR_TYPE irt, Type const* type, IN IR * opnd);
    IR * buildLogicalNot(IR * opnd0);
    IR * buildLogicalOp(IR_TYPE irt, IR * opnd0, IR * opnd1);
    IR * buildImmInt(HOST_INT v, Type const* type);
    IR * buildImmFp(HOST_FP fp, Type const* type);
    IR * buildLda(VAR * var);
    IR * buildLdaString(CHAR const* varname, SYM * string);
    IR * buildLdaString(CHAR const* varname, CHAR const * string);
    IR * buildLoad(VAR * var, Type const* type);
    IR * buildLoad(VAR * var);
    IR * buildIload(IR * base, Type const* type);
    IR * buildIload(IR * base, UINT ofst, Type const* type);
    IR * buildStore(VAR * lhs, IR * rhs);
    IR * buildStore(VAR * lhs, Type const* type, IR * rhs);
    IR * buildStore(VAR * lhs, Type const* type, UINT ofst, IR * rhs);
    IR * buildStorePR(UINT prno, Type const* type, IR * rhs);
    IR * buildStorePR(Type const* type, IR * rhs);
    IR * buildIstore(IR * base, IR * rhs, Type const* type);
    IR * buildIstore(IR * base, IR * rhs, UINT ofst, Type const* type);
    IR * buildString(SYM const* strtab);
    IR * buildStoreArray(
            IR * base,
            IR * sublist,
            Type const* type,
            Type const* elemtype,
            UINT dims,
            TMWORD const* elem_num_buf,
            IR * rhs);
    IR * buildArray(
            IR * base,
            IR * sublist,
            Type const* type,
            Type const* elemtype,
            UINT dims,
            TMWORD const* elem_num_buf);
    IR * buildReturn(IR * ret_exp);
    IR * buildSelect(IR * det, IR * true_exp, IR * false_exp, Type const* type);
    IR * buildPhi(UINT prno, Type const* type, UINT num_opnd);
    IR * buildRegion(Region * ru);
    IR * buildIcall(
            IR * callee,
            IR * param_list,
            UINT result_prno,
            Type const* type);
    IR * buildIcall(IR * callee, IR * param_list)
    { return buildIcall(callee, param_list, 0, get_type_mgr()->getVoid()); }
    IR * buildCall(
            VAR * callee,
            IR * param_list,
            UINT result_prno,
            Type const* type);
    IR * buildCall(VAR * callee,  IR * param_list)
    { return buildCall(callee, param_list, 0, get_type_mgr()->getVoid()); }

    IR * constructIRlist(bool clean_ir_list);
    void constructIRBBlist();
    HOST_INT calcIntVal(IR_TYPE ty, HOST_INT v0, HOST_INT v1);
    double calcFloatVal(IR_TYPE ty, double v0, double v1);
    size_t count_mem();
    void checkValidAndRecompute(OptCtx * oc, ...);

    virtual void destroy();
    void destroyPassMgr();
    IR * dupIR(IR const* ir);
    IR * dupIRTree(IR const* ir);
    IR * dupIRTreeList(IR const* ir);

    void dumpAllocatedIR();
    void dumpVARInRegion();
    void dumpVarMD(VAR * v, UINT indent);
    void dumpFreeTab();
    void dumpMemUsage();
    void dump(bool dump_inner_region);

    void freeIR(IR * ir);
    void freeIRTree(IR * ir);
    void freeIRTreeList(IR * ir);
    void freeIRTreeList(IRList & irs);
    IR * foldConstFloatUnary(IR * ir, bool & change);
    IR * foldConstFloatBinary(IR * ir, bool & change);
    IR * foldConstIntUnary(IR * ir, bool & change);
    IR * foldConstIntBinary(IR * ir, bool & change);
    IR * foldConst(IR * ir, bool & change);
    void findFormalParam(OUT List<VAR const*> & varlst, bool in_decl_order);

    UINT get_irt_size(IR * ir) const
    {
        #ifdef CONST_IRT_SZ
        return IR_irt_size(ir);
        #else
        return IRTSIZE(IR_code(ir));
        #endif
    }

    MDSystem * get_md_sys() const { return get_region_mgr()->get_md_sys(); }
    TypeMgr * get_type_mgr() const { return get_region_mgr()->get_type_mgr(); }
    TargInfo * get_targ_info() const
    { return get_region_mgr()->get_targ_info(); }

    SMemPool * get_pool() const
    { return REGION_analysis_instrument(this)->m_pool; }

    UINT get_pr_count() const
    { return REGION_analysis_instrument(this)->m_pr_count; }

    VAR * get_ru_var() const { return m_var; }

    inline RegionMgr * get_region_mgr() const
    {
        ASSERT0(is_function() || is_program());
        ASSERT0(REGION_analysis_instrument(this));
        return ANA_INS_ru_mgr(REGION_analysis_instrument(this));
    }

    IR * get_ir_list() const
    { return REGION_analysis_instrument(this)->m_ir_list; }

    VarMgr * get_var_mgr() const { return get_region_mgr()->get_var_mgr(); }

    VarTab * get_var_tab() const
    { return &REGION_analysis_instrument(this)->m_ru_var_tab; }

    BitSetMgr * get_bs_mgr() const
    { return &REGION_analysis_instrument(this)->m_bs_mgr; }

    DefMiscBitSetMgr * getMiscBitSetMgr() const
    { return &REGION_analysis_instrument(this)->m_sbs_mgr; }

    MDSetMgr * get_mds_mgr() const
    { return &REGION_analysis_instrument(this)->m_mds_mgr; }

    BBList * get_bb_list() const
    { return &REGION_analysis_instrument(this)->m_ir_bb_list; }

    IRBBMgr * get_bb_mgr() const
    { return &REGION_analysis_instrument(this)->m_ir_bb_mgr; }

    //Get MDSetHash.
    MDSetHash * get_mds_hash() const
    { return &REGION_analysis_instrument(this)->m_mds_hash; }

    //Return IR pointer via the unique IR_id.
    IR * get_ir(UINT irid) const
    {
        ASSERT0(REGION_analysis_instrument(this)->m_ir_vector.get(irid));
        return REGION_analysis_instrument(this)->m_ir_vector.get(irid);
    }

    //Return the vector that record all allocated IRs.
    Vector<IR*> * get_ir_vec() const
    { return &REGION_analysis_instrument(this)->m_ir_vector; }

    //Return PassMgr.
    PassMgr * get_pass_mgr() const
    { return REGION_analysis_instrument(this)->m_pass_mgr; }

    //Return IR_CFG.
    IR_CFG * get_cfg() const
    {
        return get_pass_mgr() != NULL ?
                   (IR_CFG*)get_pass_mgr()->queryPass(PASS_CFG) : NULL;
    }

    IR_AA * get_aa() const
    {
        return get_pass_mgr() != NULL ?
                (IR_AA*)get_pass_mgr()->queryPass(PASS_AA) : NULL;
    }

    //Return DU info manager.
    IR_DU_MGR * get_du_mgr() const
    {
        return get_pass_mgr() != NULL ?
                (IR_DU_MGR*)get_pass_mgr()->queryPass(PASS_DU_MGR) : NULL;
    }

    Region * get_parent() const { return REGION_parent(this); }
    CHAR const* get_ru_name() const;
    Region * get_func_unit();

    //Allocate and return all CALL in the region.
    inline List<IR const*> * gen_call_list()
    {
        if (REGION_analysis_instrument(this)->m_call_list == NULL) {
            REGION_analysis_instrument(this)->m_call_list =
                new List<IR const*>();
        }
        return REGION_analysis_instrument(this)->m_call_list;
    }

    //Read and return Call list in the Region.
    List<IR const*> * get_call_list() const
    { return REGION_analysis_instrument(this)->m_call_list; }

    //Allocate and return a list of IR_RETURN in current Region.
    inline List<IR const*> * gen_return_list()
    {
        if (REGION_analysis_instrument(this)->m_return_list == NULL) {
            REGION_analysis_instrument(this)->m_return_list =
                                        new List<IR const*>();
        }
        return REGION_analysis_instrument(this)->m_return_list;
    }

    //Return a list of IR_RETURN in current Region.
    List<IR const*> * get_return_list() const
    { return REGION_analysis_instrument(this)->m_return_list; }

    //Get the MayDef MDSet of Region.
    MDSet * get_may_def() const
    { return m_ref_info != NULL ? &REF_INFO_maydef(m_ref_info) : NULL; }

    //Get the MayUse MDSet of Region.
    MDSet * get_may_use() const
    { return m_ref_info != NULL ? &REF_INFO_mayuse(m_ref_info) : NULL; }

    Region * getTopRegion()
    {
        Region * ru = this;
        while (ru->get_parent() != NULL) {
            ru = ru->get_parent();
        }
        return ru;
    }

    //Allocate a internal LabelInfo that is not declared by compiler user.
    inline LabelInfo * genIlabel()
    {
        LabelInfo * li = genIlabel(RM_label_count(get_region_mgr()));
        RM_label_count(get_region_mgr())++;
        return li;
    }

    //Allocate a LabelInfo accroding to given 'labid'.
    LabelInfo * genIlabel(UINT labid)
    {
        ASSERT0(labid <= RM_label_count(get_region_mgr()));
        LabelInfo * li = allocInternalLabel(get_pool());
        LABEL_INFO_num(li) = labid;
        return li;
    }

    LabelInfo * genCustomLabel(SYM const* labsym)
    {
        ASSERT0(labsym);
        return allocCustomerLabel(labsym, get_pool());
    }

    //Allocate VAR for PR.
    VAR * genVARforPR(UINT prno, Type const* type);

    //Allocate MD for PR.
    MD const* genMDforPR(UINT prno, Type const* type);

    //Generate MD corresponding to PR load or write.
    MD const* genMDforPR(IR const* ir)
    {
        ASSERT0(ir->is_write_pr() || ir->is_read_pr() || ir->is_calls_stmt());
        return genMDforPR(ir->get_prno(), ir->get_type());
    }

    //Generate MD for VAR.
    MD const* genMDforVAR(VAR * var)
    { return genMDforVAR(var, var->get_type()); }


    //Generate MD for VAR.
    MD const* genMDforVAR(VAR * var, Type const* type)
    {
        ASSERT0(var && type);
        MD md;
        MD_base(&md) = var;

        if (type->is_void()) {
            MD_ty(&md) = MD_UNBOUND;
        } else {
            MD_size(&md) = get_type_mgr()->get_bytesize(type);
            MD_ty(&md) = MD_EXACT;
        }

        MD const* e = get_md_sys()->registerMD(md);
        ASSERT0(MD_id(e) > 0);
        return e;
    }

    //Generate MD for IR_ST.
    MD const* genMDforStore(IR const* ir)
    {
        ASSERT0(ir->is_st());
        return genMDforVAR(ST_idinfo(ir), ir->get_type());
    }

    //Generate MD for IR_LD.
    MD const* genMDforLoad(IR const* ir)
    {
        ASSERT0(ir->is_ld());
        return genMDforVAR(LD_idinfo(ir), ir->get_type());
    }

    //Generate MD for IR_ID.
    MD const* genMDforId(IR const* ir)
    {
        ASSERT0(ir->is_id());
        return genMDforVAR(ID_info(ir), ir->get_type());
    }

    //Return the tyid for array index, the default is unsigned 32bit.
    inline Type const* getTargetMachineArrayIndexType()
    {
        return get_type_mgr()->getSimplexTypeEx(get_type_mgr()->
            get_dtype(WORD_LENGTH_OF_TARGET_MACHINE, false));
    }

    //Perform high level optmizations.
    virtual bool HighProcess(OptCtx & oc);

    UINT id() const { return REGION_id(this); }
    
    //Initialze Region.
    void init(REGION_TYPE rt, RegionMgr * rm);

    void initRefInfo()
    {
        if (m_ref_info != NULL) { return; }
        m_ref_info = (RefInfo*)xmalloc(sizeof(RefInfo));
    }

    //Allocate and initialize pass manager.
    PassMgr * initPassMgr();

    //Invert condition for relation operation.
    virtual void invertCondition(IR ** cond);

    //Insert CVT for float if necessary.
    virtual IR * insertCvtForFloat(IR * parent, IR * kid, bool & change);

    bool isSafeToOptimize(IR const* ir);

    //Return true if ir belongs to current region.
    bool isRegionIR(IR const* ir);

    //Check and insert data type CVT if it is necessary.
    IR * insertCvt(IR * parent, IR * kid, bool & change);
    void insertCvtForBinaryOp(IR * ir, bool & change);

    //Return true if the tree height is not great than 2.
    //e.g: tree a + b is lowest height , but a + b + c is not.
    //Note that if ARRAY or ILD still not be lowered at the moment, regarding
    //it as a whole node. e.g: a[1][2] + b is the lowest height.
    bool isLowestHeight(IR const* ir, SimpCtx const* ctx) const;
    bool isLowestHeightExp(IR const* ir, SimpCtx const* ctx) const;
    bool isLowestHeightSelect(IR const* ir) const;
    bool isLowestHeightArrayOp(IR const* ir) const;

    //Return true if Region name is equivalent to 'n'.
    bool isRegionName(CHAR const* n) const
    {
        ASSERT(get_ru_name(), ("Region does not have name"));
        return strcmp(get_ru_name(), n) == 0;
    }

    bool is_undef() const { return REGION_type(this) == RU_UNDEF; }
    bool is_function() const { return REGION_type(this) == RU_FUNC; }
    bool is_blackbox() const { return REGION_type(this) == RU_BLX; }
    bool is_program() const { return REGION_type(this) == RU_PROGRAM; }
    bool is_subregion() const { return REGION_type(this) == RU_SUB; }
    bool is_eh() const { return REGION_type(this) == RU_EH; }
    bool is_readonly() const { return REGION_is_readonly(this); }

    //Perform middle level IR optimizations which are implemented
    //accroding to control flow info and data flow info.
    virtual bool MiddleProcess(OptCtx & oc);

    //Map from prno to related VAR.
    VAR * mapPR2Var(UINT prno)
    { return REGION_analysis_instrument(this)->m_prno2var.get(prno); }

    //Return the Call list of current region.
    List<IR const*> const* read_call_list() const
    { return REGION_analysis_instrument(this)->m_call_list; }

    //Peephole optimizations.
    IR * refineBand(IR * ir, bool & change);
    IR * refineBor(IR * ir, bool & change);
    IR * refineCvt(IR * ir, bool & change, RefineCtx & rc);
    IR * refineLand(IR * ir, bool & change);
    IR * refineLor(IR * ir, bool & change);
    IR * refineXor(IR * ir, bool & change);
    IR * refineAdd(IR * ir, bool & change);
    IR * refineSub(IR * ir, bool & change);
    IR * refineMul(IR * ir, bool & change, RefineCtx & rc);
    IR * refineRem(IR * ir, bool & change);
    IR * refineDiv(IR * ir, bool & change, RefineCtx & rc);
    IR * refineNe(IR * ir, bool & change, RefineCtx & rc);
    IR * refineEq(IR * ir, bool & change, RefineCtx & rc);
    IR * refineMod(IR * ir, bool & change);
    IR * refineCall(IR * ir, bool & change, RefineCtx & rc);
    IR * refineIcall(IR * ir, bool & change, RefineCtx & rc);
    IR * refineSwitch(IR * ir, bool & change, RefineCtx & rc);
    IR * refineReturn(IR * ir, bool & change, RefineCtx & rc);
    IR * refinePhi(IR * ir, bool & change, RefineCtx & rc);
    IR * refineBr(IR * ir, bool & change, RefineCtx & rc);
    IR * refineSelect(IR * ir, bool & change, RefineCtx & rc);
    IR * refineBranch(IR * ir);
    IR * refineArray(IR * ir, bool & change, RefineCtx & rc);
    IR * refineNeg(IR * ir, bool & change);
    IR * refineNot(IR * ir, bool & change, RefineCtx & rc);
    IR * refineBinaryOp(IR * ir, bool & change, RefineCtx & rc);
    IR * refineLoad(IR * ir);
    IR * refineIload1(IR * ir, bool & change);
    IR * refineIload2(IR * ir, bool & change);
    IR * refineIload(IR * ir, bool & change, RefineCtx & rc);
    IR * refineDetViaSSAdu(IR * ir, bool & change);
    IR * refineDet(IR * ir_list, bool & change, RefineCtx & rc);
    IR * refineStore(IR * ir, bool & change, RefineCtx & rc);
    IR * refineStoreArray(IR * ir, bool & change, RefineCtx & rc);
    IR * refineIstore(IR * ir, bool & change, RefineCtx & rc);
    IR * refineIR(IR * ir, bool & change, RefineCtx & rc);
    IR * refineIRlist(IR * ir_list, bool & change, RefineCtx & rc);
    bool refineStmtList(IN OUT BBIRList & ir_list, RefineCtx & rc);
    bool refineBBlist(IN OUT BBList * ir_bb_list, RefineCtx & rc);
    IR * reassociation(IR * ir, bool & change);
    bool reconstructBBlist(OptCtx & oc);

    IR * simpToPR(IR * ir, SimpCtx * ctx);
    C<IRBB*> * splitIRlistIntoBB(IR * irs, BBList * bbl, C<IRBB*> * ctbb);
    IR * simplifyLoopIngredient(IR * ir, SimpCtx * ctx);
    IR * simplifyBranch(IR * ir, SimpCtx * ctx);
    IR * simplifyIfSelf(IR * ir, SimpCtx * ctx);
    IR * simplifyDoWhileSelf(IR * ir, SimpCtx * ctx);
    IR * simplifyWhileDoSelf(IR * ir, SimpCtx * ctx);
    IR * simplifyDoLoopSelf(IR * ir, SimpCtx * ctx);
    IR * simplifySwitchSelf(IR * ir, SimpCtx * ctx);
    void simplifySelectKids(IR * ir, SimpCtx * cont);
    IR * simplifyStore(IR * ir, SimpCtx * cont);
    IR * simplifyStorePR(IR * ir, SimpCtx * cont);
    IR * simplifyArrayIngredient(IR * ir, SimpCtx * ctx);
    IR * simplifyStoreArray(IR * ir, SimpCtx * ctx);
    IR * simplifySetelem(IR * ir, SimpCtx * ctx);
    IR * simplifyGetelem(IR * ir, SimpCtx * ctx);
    IR * simplifyIstore(IR * ir, SimpCtx * cont);
    IR * simplifyCall(IR * ir, SimpCtx * cont);
    IR * simplifyIf (IR * ir, SimpCtx * cont);
    IR * simplifyWhileDo(IR * ir, SimpCtx * cont);
    IR * simplifyDoWhile (IR * ir, SimpCtx * cont);
    IR * simplifyDoLoop(IR * ir, SimpCtx * cont);
    IR * simplifyDet(IR * ir, SimpCtx * cont);
    IR * simplifyJudgeDet(IR * ir, SimpCtx * cont);
    IR * simplifySelect(IR * ir, SimpCtx * cont);
    IR * simplifySwitch (IR * ir, SimpCtx * cont);
    IR * simplifyIgoto(IR * ir, SimpCtx * cont);
    IR * simplifyArrayAddrExp(IR * ir, SimpCtx * cont);
    IR * simplifyArray(IR * ir, SimpCtx * cont);
    IR * simplifyExpression(IR * ir, SimpCtx * cont);
    IR * simplifyBinAndUniExpression(IR * ir, SimpCtx * ctx);
    IR * simplifyStmt(IR * ir, SimpCtx * cont);
    IR * simplifyStmtList(IR * ir, SimpCtx * cont);
    void simplifyBB(IRBB * bb, SimpCtx * cont);
    void simplifyBBlist(BBList * bbl, SimpCtx * cont);
    IR * simplifyLogicalNot(IN IR * ir, SimpCtx * cont);
    IR * simplifyLogicalOrAtFalsebr(IN IR * ir, LabelInfo const* tgt_label);
    IR * simplifyLogicalOrAtTruebr(IN IR * ir, LabelInfo const* tgt_label);
    IR * simplifyLogicalOr(IN IR * ir, SimpCtx * cont);
    IR * simplifyLogicalAndAtTruebr(IN IR * ir, LabelInfo const* tgt_label);
    IR * simplifyLogicalAndAtFalsebr(IN IR * ir, LabelInfo const* tgt_label);
    IR * simplifyLogicalAnd(IN IR * ir, SimpCtx * cont);
    IR * simplifyLogicalDet(IR * ir, SimpCtx * cont);
    void set_irt_size(IR * ir, UINT)
    {
        #ifdef CONST_IRT_SZ
        IR_irt_size(ir) = irt_sz;
        #else
        UNUSED(ir);
        #endif
    }
    void setMapPR2Var(UINT prno, VAR * pr_var)
    { REGION_analysis_instrument(this)->m_prno2var.set(prno, pr_var); }

    void set_ru_var(VAR * v) { m_var = v; }
    void set_ir_list(IR * irs) { REGION_analysis_instrument(this)->m_ir_list = irs; }
    void set_blx_data(void * d) { REGION_blx_data(this) = d; }
    IR * StrengthReduce(IN OUT IR * ir, IN OUT bool & change);

    //num_inner_region: count the number of inner regions.
    void scanCallAndReturnList(
            OUT UINT & num_inner_region,
            OUT List<IR const*> * call_list,
            OUT List<IR const*> * ret_list,
            bool scan_inner_region);
    void scanCallAndReturnList(
            OUT UINT & num_inner_region,
            bool scan_inner_region)
    {
        gen_call_list()->clean();
        gen_return_list()->clean(); //Scan RETURN as well.
        scanCallAndReturnList(num_inner_region, get_call_list(),
                              get_return_list(), scan_inner_region);
    }

    void lowerIRTreeToLowestHeight(OptCtx & oc);

    virtual bool process(OptCtx * oc); //Entry to process region.

    //Check and rescan call list of region if one of elements in list changed.
    void updateCallAndReturnList(bool scan_inner_region);

    bool verifyBBlist(BBList & bbl);
    bool verifyIRinRegion();
    bool verifyRPO(OptCtx & oc);
    bool verifyMDRef();
};
//END Region

} //namespace xoc
#endif
