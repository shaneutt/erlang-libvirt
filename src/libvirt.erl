%% Copyright (c) 2010-2011, Michael Santos <michael.santos@gmail.com>
%% All rights reserved.
%% 
%% Redistribution and use in source and binary forms, with or without
%% modification, are permitted provided that the following conditions
%% are met:
%% 
%% Redistributions of source code must retain the above copyright
%% notice, this list of conditions and the following disclaimer.
%% 
%% Redistributions in binary form must reproduce the above copyright
%% notice, this list of conditions and the following disclaimer in the
%% documentation and/or other materials provided with the distribution.
%% 
%% Neither the name of the author nor the names of its contributors
%% may be used to endorse or promote products derived from this software
%% without specific prior written permission.
%% 
%% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
%% "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
%% LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
%% FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
%% COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
%% INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
%% BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
%% LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
%% CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
%% LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
%% ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
%% POSSIBILITY OF SUCH DAMAGE.
-module(libvirt).
-include("libvirt.hrl").

-export([
        cast/2, cast/3, cast/4,
        call/1, call/2,

        open/1,
        close/1,
        get/2,
        set/2,

        define/2,
        undefine/1,
        create/1, create/2,
        destroy/1,

        save/2,
        restore/2,
        shutdown/1,
        suspend/1,
        resume/1,

        resource/2
    ]).


-on_load(on_load/0).

on_load() ->
    erlang:load_nif(niflib(), []).


%%-------------------------------------------------------------------------
%%% NIF stubs
%%-------------------------------------------------------------------------
cast(_,_) ->
    erlang:error(not_implemented).
cast(_,_,_) ->
    erlang:error(not_implemented).
cast(_,_,_,_) ->
    erlang:error(not_implemented).

%%-------------------------------------------------------------------------
%%% Blocking API
%%-------------------------------------------------------------------------
call({Fun, Arg}, Timeout) ->
    block(cast(Fun, Arg), Timeout);
call({Fun, Arg1, Arg2}, Timeout) ->
    block(cast(Fun, Arg1, Arg2), Timeout);
call({Fun, Arg1, Arg2, Arg3}, Timeout) ->
    block(cast(Fun, Arg1, Arg2, Arg3), Timeout).

call(Arg) ->
    call(Arg, infinity).

block(ok, Timeout) ->
    receive
        Res -> Res
    after
        Timeout -> {error, timeout}
    end;
block(Error, _Timeout) ->
    Error.


%%
%% Connections
%%
open(Name) when is_list(Name) ->
    open({connect, Name});

open({connect, Name}) when is_list(Name) ->
    call({connect_open, Name, ?VERT_CONNECT_OPEN});
open({connect, Name, read}) when is_list(Name) ->
    call({connect_open,Name, ?VERT_CONNECT_OPEN_READONLY});
open({connect, Name, {auth, Options}}) when is_list(Name), is_list(Options) ->
%    call({connect_open, Name, ?VERT_CONNECT_OPEN_AUTH}).
    erlang:error(not_implemented).

close(#resource{type = connect, res = Res}) ->
    call({connect_close, Res}).

define(#resource{type = connect, res = Res}, {Type, Cfg}) when is_atom(Type),
    ( is_list(Cfg) orelse is_binary(Cfg) ) ->
    call({resource_define, Res, res(Type), Cfg}).

undefine(#resource{res = Res}) ->
    call({resource_undefine, Res}).

create(Resource) ->
    create(Resource, 0).
create(#resource{res = Res}, Flags) when is_integer(Flags) ->
    call({resource_create, Res, Flags}).


%%
%% Resources
%%
destroy(Resource) ->
    resource(Resource, destroy).

save(Resource, File) ->
    resource(Resource, {save, File}).

restore(Resource, File) ->
    resource(Resource, {restore, File}).

shutdown(Resource) ->
    resource(Resource, shutdown).

suspend(Resource) ->
    resource(Resource, suspend).

resume(Resource) ->
    resource(Resource, resume).


%%-------------------------------------------------------------------------
%%% Resources
%%-------------------------------------------------------------------------

%%
%% connect
%%
resource(#resource{type = connect, res = Res}, {domain, {Attr, Arg}}) when is_atom(Attr) ->
    call({domain_lookup, Res, attr(Attr), Arg});

resource(#resource{type = connect, res = Res}, {interface, {Attr, Arg}}) when is_list(Attr) ->
    call({interface_lookup, Res, attr(Attr), Arg});

resource(#resource{type = connect, res = Res}, {network, {Attr, Arg}}) when is_list(Attr) ->
    call({network_lookup, Res, attr(Attr), Arg});

resource(#resource{type = connect, res = Res}, {Type, num_active}) when is_atom(Type) ->
    call({connect_get_numactive, Res, res(Type)});
resource(#resource{type = connect, res = Res}, {Type, num_inactive}) when is_atom(Type) ->
    call({connect_get_numinactive, Res, res(Type)});

resource(#resource{type = connect, res = Res}, {Type, active}) ->
    Resource = res(Type),
    case call({connect_get_numactive, Res, Resource}) of
        {ok, 0} -> [];
        {ok, Max} -> call({connect_get_listactive, Res, Resource, Max});
        Err -> Err
    end;
resource(#resource{type = connect, res = Res}, {Type, inactive}) ->
    Resource = res(Type),
    case call({connect_get_numactive, Res, Resource}) of
        {ok, 0} -> [];
        {ok, Max} -> call({connect_get_listinactive, Res, Resource, Max});
        Err -> Err
    end;

%%
%% Domain
%%
resource(#resource{type = domain, res = Res}, {save, File}) when is_list(File) ->
    call({domain_save, Res, File});

resource(#resource{type = connect, res = Res}, {resource, File}) when is_list(File) ->
    call({domain_restore, Res, File});

resource(#resource{type = domain, res = Res}, shutdown) ->
    call({domain_shutdown, Res});
resource(#resource{type = domain, res = Res}, suspend) ->
    call({domain_suspend, Res});
resource(#resource{type = domain, res = Res}, resume) ->
    call({domain_resume, Res});

%%
%% Resource
%%
resource(#resource{res = Res}, destroy) ->
    call({resource_destroy, Res}).


%%-------------------------------------------------------------------------
%%% Resource attributes
%%-------------------------------------------------------------------------
get(#resource{type = connect, res = Res} = Conn, cellsfreememory) ->
    {ok, #node_info{nodes = Nodes}} = ?MODULE:get(Conn, info),
    call({connect_get, Res, attr(cellsfreememory), Nodes});

%% struct _virNodeInfo {
%%     char model[32];     /* string indicating the CPU model */
%%     unsigned long memory;/* memory size in kilobytes */
%%     unsigned int cpus;  /* the number of active CPUs */
%%     unsigned int mhz;   /* expected CPU frequency */
%%     unsigned int nodes; /* the number of NUMA cell, 1 for uniform mem access */
%%     unsigned int sockets;/* number of CPU socket per node */
%%     unsigned int cores; /* number of core per socket */
%%     unsigned int threads;/* number of threads per core */
%% };
get(#resource{type = connect, res = Res}, info) ->
    Long = erlang:system_info(wordsize),
    case call({connect_get, Res, attr(info)}) of
        {ok, <<
            Model:32/native-bytes,
            Memory:Long/native-unsigned-integer-unit:8,
            Cpus:32/native,
            Mhz:32/native,
            Nodes:32/native,
            Sockets:32/native,
            Cores:32/native,
            Threads:32/native
        >>} ->
        {ok, #node_info{
                model = Model,
                memory = Memory,
                cpus = Cpus,
                mhz = Mhz,
                nodes = Nodes,
                sockets = Sockets,
                cores = Cores,
                threads = Threads
            }};
        Error ->
            Error
    end;
%% struct virDomainInfo{
%%     unsigned char   state
%%     unsigned long   maxMem (Kb)
%%     unsigned long   memory (Kb)
%%     unsigned short  nrVirtCpu
%%     unsigned long long  cpuTime (nanoseconds) 
%% }
get(#resource{type = domain, res = Res}, info) ->
    Long = erlang:system_info({wordsize, external}),
    case call({domain_get, Res, attr(info)}) of
        {ok, <<
            State:8, % _Pad:24,
            MaxMem:Long/native-unsigned-integer-unit:8,
            Memory:Long/native-unsigned-integer-unit:8,
            NrVirtCpu:2/native-unsigned-integer-unit:8, % _Pad1:16,
            CpuTime:8/native-unsigned-integer-unit:8
            >>} ->
            #domain_info{
                state = state({domain, State}),
                maxmem = MaxMem,
                memory = Memory,
                nrvirtcpu = NrVirtCpu,
                cputime = CpuTime
            };
        Err ->
            Err
    end;

get(#resource{type = connect, res = Res}, libversion) ->
    case call({connect_get, Res, attr(libversion)}) of
        {ok, Version} ->
            {ok, version(Version)};
        Err ->
            Err
    end;
get(#resource{type = connect, res = Res}, version) ->
    case call({connect_get, Res, attr(version)}) of
        {ok, Version} ->
            {ok, version(Version)};
        Err ->
            Err
    end;

get(#resource{type = connect, res = Res}, maxvcpus) ->
    call({connect_get, Res, attr(maxvcpus), []});
get(#resource{type = connect, res = Res}, {maxvcpus, Type}) when is_list(Type) ->
    call({connect_get, Res, attr(maxvcpus), Type});

%% struct _virSecurityModel {
%%  char model[VIR_SECURITY_MODEL_BUFLEN];      /* security model string */
%%  char doi[VIR_SECURITY_DOI_BUFLEN];          /* domain of interpetation */
%% }
get(#resource{type = connect, res = Res}, securitymodel) ->
    case call({connect_get, Res, attr(securitymodel)}) of
        {ok, <<
            Model:?VIR_SECURITY_MODEL_BUFLEN/native-bytes,
            Doi:?VIR_SECURITY_DOI_BUFLEN/native-bytes
            >>} ->
            {ok, #security_model{
                    model = Model,
                    doi = Doi
                }};
        Err ->
            Err
    end;

get(#resource{type = connect, res = Res}, {Attr, Arg}) when is_atom(Attr) ->
    call({connect_get, Res, attr(Attr), Arg});
get(#resource{type = connect, res = Res}, Attr) when is_atom(Attr) ->
    call({connect_get, Res, attr(Attr)});

get(#resource{type = domain, res = Res}, {Attr, Arg}) when is_atom(Attr) ->
    call({domain_get, Res, attr(Attr), Arg});
get(#resource{type = domain, res = Res}, Attr) when is_atom(Attr) ->
    call({domain_get, Res, attr(Attr)});

get(#resource{type = interface, res = Res}, {Attr, Arg}) when is_atom(Attr) ->
    call({domain_get, Res, attr(Attr), Arg});
get(#resource{type = interface, res = Res}, Attr) when is_atom(Attr) ->
    call({interface_get, Res, attr(Attr)});

get(#resource{type = network, res = Res}, {Attr, Arg}) when is_atom(Attr) ->
    call({network_get, Res, attr(Attr), Arg});
get(#resource{type = network, res = Res}, Attr) when is_atom(Attr) ->
    call({network_get, Res, attr(Attr)}).

set(Resource, autostart) ->
    call({set, Resource, {autostart, true}});
set(#resource{type = domain, res = Res}, {autostart, true}) ->
    call({domain_set_autostart, Res, 1});
set(#resource{type = domain, res = Res}, {autostart, false}) ->
    call({domain_set_autostart, Res, 0}).




%%-------------------------------------------------------------------------
%%% Internal functions
%%-------------------------------------------------------------------------
version(Version) when is_integer(Version) ->
    Major = Version div 1000000,
    Minor = Version rem 1000000 div 1000,
    Release = Version rem 1000000 rem 1000,
    {Major, Minor, Release}.

res(domain) -> ?VERT_RES_DOMAIN;
res(interface) -> ?VERT_RES_INTERFACE;
res(network) -> ?VERT_RES_NETWORK;
res(storagepool) -> ?VERT_RES_STORAGEPOOL;
res(filter) -> ?VERT_RES_FILTER;
res(secret) -> ?VERT_RES_SECRET.

attr(id) -> ?VERT_ATTR_ID;
attr(name) -> ?VERT_ATTR_NAME;
attr(uuid) -> ?VERT_ATTR_UUID;
attr(rawuuid) -> ?VERT_ATTR_RAWUUID;
attr(mac) -> ?VERT_ATTR_MAC;
attr(desc) -> ?VERT_ATTR_DESC;
attr(info) -> ?VERT_ATTR_INFO;
attr(autostart) -> ?VERT_ATTR_AUTOSTART;
attr(blockinfo) -> ?VERT_ATTR_BLOCKINFO;
attr(connect) -> ?VERT_ATTR_CONNECT;
attr(jobinfo) -> ?VERT_ATTR_JOBINFO;
attr(maxmemory) -> ?VERT_ATTR_MAXMEMORY;
attr(maxvcpus) -> ?VERT_ATTR_MAXVCPUS;
attr(memoryparameters) -> ?VERT_ATTR_MEMORYPARAMETERS;
attr(ostype) -> ?VERT_ATTR_OSTYPE;
attr(scheduleparatmers) -> ?VERT_ATTR_SCHEDULERPARAMETERS;
attr(schedulertype) -> ?VERT_ATTR_SCHEDULERTYPE;
attr(securitylabel) -> ?VERT_ATTR_SECURITYLABEL;
attr(vcpus) -> ?VERT_ATTR_VCPUS;
attr(capabilities) -> ?VERT_ATTR_CAPABILITIES;
attr(hostname) -> ?VERT_ATTR_HOSTNAME;
attr(libversion) -> ?VERT_ATTR_LIBVERSION;
attr(freememory) -> ?VERT_ATTR_FREEMEMORY;
attr(cellsfreememory) -> ?VERT_ATTR_CELLSFREEMEMORY;
attr(type) -> ?VERT_ATTR_TYPE;
attr(version) -> ?VERT_ATTR_VERSION;
attr(uri) -> ?VERT_ATTR_URI;
attr(encrypted) -> ?VERT_ATTR_ENCRYPTED;
attr(secure) -> ?VERT_ATTR_SECURE;
attr(securitymodel) -> ?VERT_ATTR_SECURITYMODEL;
attr(bridgename) -> ?VERT_ATTR_BRIDGENAME;
attr(active) -> ?VERT_ATTR_ACTIVE;
attr(persistent) -> ?VERT_ATTR_PERSISTENT.

state({domain, ?VIR_DOMAIN_NOSTATE}) -> undefined;
state({domain, ?VIR_DOMAIN_RUNNING}) -> running;
state({domain, ?VIR_DOMAIN_BLOCKED}) -> blocked;
state({domain, ?VIR_DOMAIN_PAUSED}) -> paused;
state({domain, ?VIR_DOMAIN_SHUTDOWN}) -> shutdown;
state({domain, ?VIR_DOMAIN_SHUTOFF}) -> shutoff;
state({domain, ?VIR_DOMAIN_CRASHED}) -> crashed;

state({domain, undefined}) -> ?VIR_DOMAIN_NOSTATE;
state({domain, running}) -> ?VIR_DOMAIN_RUNNING;
state({domain, blocked}) -> ?VIR_DOMAIN_BLOCKED;
state({domain, paused}) -> ?VIR_DOMAIN_PAUSED;
state({domain, shutdown}) -> ?VIR_DOMAIN_SHUTDOWN;
state({domain, shutoff}) -> ?VIR_DOMAIN_SHUTOFF;
state({domain, crashed}) -> ?VIR_DOMAIN_CRASHED.


privdir(File) ->
    filename:join([
        filename:dirname(code:which(?MODULE)),
        "..",
        "priv",
        File
    ]).

niflib() ->
    privdir(?MODULE).
