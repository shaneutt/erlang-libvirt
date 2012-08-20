/* Copyright (c) 2010-2012, Michael Santos <michael.santos@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the author nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "vert.h"
#include "vert_util.h"
#include "vert_domain.h"


VERT_FUN_INT_RES(virDomainGetMaxVcpus, VERT_RES_DOMAIN)
VERT_FUN_INT_RES(virDomainShutdown, VERT_RES_DOMAIN)
VERT_FUN_INT_RES(virDomainSuspend, VERT_RES_DOMAIN)
VERT_FUN_INT_RES(virDomainResume, VERT_RES_DOMAIN)
VERT_FUN_INT_RES(virDomainUndefine, VERT_RES_DOMAIN)
VERT_FUN_INT_RES(virDomainDestroy, VERT_RES_DOMAIN)

#if HAVE_VIRDOMAINCREATEWITHFLAGS
    ERL_NIF_TERM
vert_virDomainCreate(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    return vert_domain_int_res_int(env, argv, virDomainCreateWithFlags);
}
#else
VERT_FUN_INT_RES(virDomainCreate, VERT_RES_DOMAIN)
#endif


    ERL_NIF_TERM
vert_virDomainLookupByID(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *vp = NULL;
    VERT_RESOURCE *dp = NULL;
    int id = 0;


    VERT_GET_RESOURCE(0, vp, VERT_RES_CONNECT);
    VERT_GET_INT(1, id);

    VERT_RES_ALLOC(dp, VERT_RES_DOMAIN, vp->res);

    dp->res = virDomainLookupByID(vp->res, id);

    if (dp->res == NULL) {
        enif_release_resource(dp);
        return verterr(env);
    }

    return vert_make_resource(env, dp, atom_domain);
}

    ERL_NIF_TERM
vert_virDomainLookupByName(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{

    VERT_RESOURCE *vp = NULL;
    VERT_RESOURCE *dp = NULL;
    ErlNifBinary buf = {0};


    VERT_GET_RESOURCE(0, vp, VERT_RES_CONNECT);
    VERT_GET_IOLIST(1, buf);

    VERT_BIN_APPEND_NULL(buf);

    VERT_RES_ALLOC(dp, VERT_RES_DOMAIN, vp->res);

    dp->res = virDomainLookupByName(vp->res, (char *)buf.data);

    if (dp->res == NULL) {
        enif_release_resource(dp);
        return verterr(env);
    }

    return vert_make_resource(env, dp, atom_domain);
}

    ERL_NIF_TERM
vert_virDomainLookupByUUID(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *vp = NULL;
    VERT_RESOURCE *dp = NULL;
    ErlNifBinary buf = {0};


    VERT_GET_RESOURCE(0, vp, VERT_RES_CONNECT);
    VERT_GET_IOLIST(1, buf);

    VERT_BIN_APPEND_NULL(buf);

    dp->res = virDomainLookupByUUID(vp->res, buf.data);

    if (dp->res == NULL) {
        enif_release_resource(dp);
        return verterr(env);
    }

    return vert_make_resource(env, dp, atom_domain);
}

    ERL_NIF_TERM
vert_virDomainGetAutostart(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    int autostart = 0;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetAutostart(dp->res, &autostart) < 0);

    return (autostart ? atom_true : atom_false);
}

#ifdef HAVE_VIRDOMAINGETBLOCKINFO
    ERL_NIF_TERM
vert_virDomainGetBlockInfo(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    ErlNifBinary path = {0};
    virDomainInfo info = {0};
    ERL_NIF_TERM buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);
    VERT_GET_IOLIST(1, buf);

    VERT_BIN_APPEND_NULL(buf);

    VERTERR(virDomainGetBlockInfo(dp->res, path.data, &info, 0) < 0);

    BINCOPY(buf, &info, sizeof(virDomainInfo));

    return enif_make_tuple2(env, atom_ok, buf);
}
#else
VERT_FUN_UNSUPPORTED(virDomainGetBlockInfo)
#endif

    ERL_NIF_TERM
vert_virDomainGetID(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    unsigned int id = 0;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    id = virDomainGetID(dp->res);

    VERTERR(id == (unsigned int)-1);

    return enif_make_tuple2(env,
            atom_ok,
            enif_make_uint(env, id));
}

    ERL_NIF_TERM
vert_virDomainGetInfo(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    virDomainInfo info = {0};
    ERL_NIF_TERM buf = {0};

    struct domain_info {
        unsigned char state;
        unsigned long maxMem;
        unsigned long memory;
        unsigned short nrVirtCpu;
        unsigned long long cpuTime;
    } __attribute__((__packed__));
    struct domain_info dip = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetInfo(dp->res, &info) < 0);

    dip.state = info.state;
    dip.maxMem = info.maxMem;
    dip.memory = info.memory;
    dip.nrVirtCpu = info.nrVirtCpu;
    dip.cpuTime = info.cpuTime;

    BINCOPY(buf, &dip, sizeof(struct domain_info));

    return enif_make_tuple2(env, atom_ok, buf);
}

    ERL_NIF_TERM
vert_virDomainGetJobInfo(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
#ifdef HAVE_VIRDOMAINGETJOBINFO
    VERT_RESOURCE *dp = NULL;
    virDomainJobInfo info = {0};
    ERL_NIF_TERM buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetJobInfo(dp->res, &info) < 0);

    BINCOPY(buf, &info, sizeof(virDomainInfo));

    return enif_make_tuple2(env, atom_ok, buf);
#else
    return error_tuple(env, atom_unsupported);
#endif
}

    ERL_NIF_TERM
vert_virDomainGetMaxMemory(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    unsigned long mem = 0;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    mem = virDomainGetMaxMemory(dp->res);    // XXX can also be NULL for domain0

    VERTERR(mem == 0);

    return enif_make_tuple2(env,
            atom_ok,
            enif_make_ulong(env, mem));
}

    ERL_NIF_TERM
vert_virDomainGetMemoryParameters(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
#ifdef HAVE_VIRDOMAINGETMEMORYPARAMETERS
    VERT_RESOURCE *dp = NULL;
    int n = 0;
    ErlNifBinary buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR( (virDomainGetMemoryParameters(dp->res, NULL, &n, 0) < 0) || n == 0);

    if (!enif_alloc_binary(sizeof(virMemoryParameter)*n, &buf))
        return atom_enomem;

    VERTERR(virDomainGetMemoryParameters(dp->res, buf.data, &n, 0) < 0);

    return enif_make_tuple2(env,
            atom_ok,
            enif_make_tuple4(env,
                enif_make_atom(env, "parameter"),
                erl_make_binary(env, &buf),
                erl_make_int(env, n)
            ));
#else
    return error_tuple(env, atom_unsupported);
#endif
}

    ERL_NIF_TERM
vert_virDomainGetName(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    const char *name = NULL;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    name = virDomainGetName(dp->res);
    VERTERR(name == NULL);

    return enif_make_tuple2(env,
            atom_ok,
            enif_make_string(env, name, ERL_NIF_LATIN1));
}

    ERL_NIF_TERM
vert_virDomainGetOSType(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    char *name = NULL;  /* should be freed */

    ERL_NIF_TERM term = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    name = virDomainGetOSType(dp->res);
    VERTERR(name == NULL);

    term = enif_make_string(env, name, ERL_NIF_LATIN1);
    free(name);

    return enif_make_tuple2(env,
            atom_ok,
            term);
}

    ERL_NIF_TERM
vert_virDomainGetSchedulerParameters(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    virSchedParameter params;
    int n = 0;
    ERL_NIF_TERM buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetSchedulerParameters(dp->res, &params, &n) < 0);

    BINCOPY(buf, &params, sizeof(virSchedParameter));

    return enif_make_tuple2(env, atom_ok, buf);
}

    ERL_NIF_TERM
vert_virDomainGetSchedulerType(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    char *name = NULL;
    int n = 0;
    ERL_NIF_TERM buf = {0};

    ERL_NIF_TERM term = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    name = virDomainGetSchedulerType(dp->res, &n);

    VERTERR(name == NULL);
    BINCOPY(buf, name, strlen(name));

    term = enif_make_tuple2(env,
        atom_ok,
        enif_make_tuple3(env,
            enif_make_atom(env, "parameter"),
            buf,
            enif_make_int(env, n)
        ));

    free(name);

    return term;
}

    ERL_NIF_TERM
vert_virDomainGetSecurityLabel(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    virSecurityLabel label;
    ERL_NIF_TERM buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetSecurityLabel(dp->res, &label) < 0);

    BINCOPY(buf, &label, sizeof(virSecurityLabel));

    return enif_make_tuple2(env, atom_ok, buf);
}

    ERL_NIF_TERM
vert_virDomainGetUUID(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    unsigned char uuid[VIR_UUID_BUFLEN];
    ERL_NIF_TERM buf = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetUUID(dp->res, uuid) < 0);
    BINCOPY(buf, &uuid, sizeof(VIR_UUID_BUFLEN));

    return enif_make_tuple2(env, atom_ok, buf);
}

    ERL_NIF_TERM
vert_virDomainGetUUIDString(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    char uuid[VIR_UUID_STRING_BUFLEN];


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetUUIDString(dp->res, uuid) < 0);

    return enif_make_tuple2(env, atom_ok,
        enif_make_string(env, uuid, ERL_NIF_LATIN1));
}

#ifdef HAVE_VIRDOMAINGETVCPUS
/*
    ERL_NIF_TERM
vert_virDomainGetVcpus(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    virVcpuInfo info = {0};
    int max = 0;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);

    VERTERR(virDomainGetVcpus(*dp, &info, maxinfo, cpumaps, int maplen) < 0);
}
*/
#endif

    ERL_NIF_TERM
vert_virDomainGetXMLDesc(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    char *desc = NULL;
    int flags = 0;

    ERL_NIF_TERM term = {0};


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);
    VERT_GET_INT(1, flags);

    desc = virDomainGetXMLDesc(dp->res, flags);

    VERTERR(desc == NULL);

    term = enif_make_tuple2(env, atom_ok,
        enif_make_string(env, desc, ERL_NIF_LATIN1));

    free(desc);

    return term;
}

    ERL_NIF_TERM
vert_virDomainSave(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    ErlNifBinary buf = {0};

    int n = -1;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);
    VERT_GET_IOLIST(1, buf);

    n = virDomainSave(dp->res, (const char *)buf.data);

    VERTERR(n < 0);

    return atom_ok;
}

    ERL_NIF_TERM
vert_virDomainRestore(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *vp = NULL;
    ErlNifBinary buf = {0};

    int n = -1;


    VERT_GET_RESOURCE(0, vp, VERT_RES_CONNECT);
    VERT_GET_IOLIST(1, buf);

    n = virDomainRestore(vp->res, (const char *)buf.data);

    VERTERR(n < 0);

    return atom_ok;
}

    ERL_NIF_TERM
vert_virDomainDefineXML(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *vp = NULL;
    ErlNifBinary cfg = {0};

    VERT_RESOURCE *dp = NULL;


    VERT_GET_RESOURCE(0, vp, VERT_RES_CONNECT);
    VERT_GET_IOLIST(1, cfg);

    VERT_BIN_APPEND_NULL(cfg);

    VERT_RES_ALLOC(dp, VERT_RES_DOMAIN, vp->res);

    dp->res = virDomainDefineXML(vp->res, (const char *)cfg.data);

    if (dp->res == NULL) {
        enif_release_resource(dp);
        return verterr(env);
    }

    return vert_make_resource(env, dp, atom_domain);
}

    ERL_NIF_TERM
vert_virDomainSetAutostart(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    return vert_domain_int_res_int(env, argv, virDomainSetAutostart);
}

    ERL_NIF_TERM
vert_virDomainOpenConsole(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    VERT_RESOURCE *dp = NULL;
    VERT_RESOURCE *sp = NULL;
    ErlNifBinary devname = {0};

    int n = -1;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);
    VERT_GET_IOLIST(1, devname);
    VERT_GET_RESOURCE(2, sp, VERT_RES_STREAM);

    if (devname.size > 0)
        VERT_BIN_APPEND_NULL(devname);

    n = virDomainOpenConsole(dp->res,
            (devname.size == 0 ? NULL : (char *)devname.data), sp->res, 0);

    VERTERR(n < 0);

    return atom_ok;
}


/*
 * Internal functions
 */
    ERL_NIF_TERM
vert_domain_int_res_int(
        ErlNifEnv *env,
        const ERL_NIF_TERM argv[],
        int (*fp)(virDomainPtr, int))
{
    VERT_RESOURCE *dp = NULL;
    int flags = 0;

    int n = -1;


    VERT_GET_RESOURCE(0, dp, VERT_RES_DOMAIN);
    VERT_GET_INT(1, flags);

    n = fp(dp->res, flags);

    VERTERR(n < 0);

    return atom_ok;
}
