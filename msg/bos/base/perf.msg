$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)48        1.2  src/msg/bos/base/perf.msg, bosmsg, msg411, 9430C411a 7/23/94 12:25:41
$
$   COMPONENT_NAME: libncs
$
$   FUNCTIONS:
$
$   ORIGINS: 84
$
$
$ -----------------------------------------------------------------------------
$
$ perf.msg
$ 
$ Message source object file used by the gencat utility for message catalogue
$ generation.  It is a compilation of catalogued messages that were extracted 
$ from the source code modules that comprise the following executables:
$
$						client
$						server
$
$ -----------------------------------------------------------------------------
$
$quote "
$delset 4
$set 4
$
$ ------------------------------------ Client Tests ---------------------------
$
$ This is a test that tests NCS.  This is the client part of that test.
$ The first 21 messages are DEVELOP messages
$
1 "Null call"
2 "host passes calls/pass verify? idempotent?"
3 "Variable length input arg"
4 "host passes calls/pass verify? idempotent? nbytes"
5 "Variable length output arg"
6 "Broadcast"
7 "family"
8 "Maybe"
9 "host"
10 "Broadcast/maybe"
11 "Floating point"
12 "Unregistered interface"
13 "Forwarding"
14 "host global?"
15 "Exception"
16 "Slow call"
17 "host passes calls/pass verify? idempotent? seconds [mode]"
18 "Shutdown"
19 "Callback"
20 "host passes callbacks/pass idempotent?"
21 "Generic interface"
$
$ The next 2 messages are NOTRANS.
$
$ %1 is the number of the test, %2 is the character string used in that particular test
22 "usage: client [-dxlsmai] [-m nthreads] %%1$d %%2$s\n"
23 "usage: client [-dxlsmai] [-m nthreads] test# ...\n"
$
$ The next 7 messages are DEVELOP messages.
$
24 "  -d: Turns on NCK runtime debug output\n"
25 "  -x: Turns on NCK runtime max debug output\n"
26 "  -l: Causes packets to be artificially lost\n"
27 "  -s: Turns on socket debug output (MS/DOS only)\n"
28 "  -m: Causes nthreads tasks to run at same time (tasking systems only)\n"
29 "  -a: Turns on asynchronous ack mode (non-tasking systems only)\n"
30 "  -i: Causes statistics to be dumped at end of run\n"
$
$ NOTRANS message
31 "\n"
$
$ The next 10 messages are DEVELOP.
$
$ %1 and %3 are the test number and %2 is the type of test and %4 is a character string used in the test
32 "  test #%%1$d: %%2$s test\n    usage: client %%3$d %%4$s\n"
$
$ %1 is a name inputed from caller, %2 is reason for failure
33 "*** Can't convert name \"%%1$s\" to sockaddr - %%2$s\n"
34 "*** Can't bind - %%1$s\n"
35 "*** Unknown family name \"%%1$s\"\n"
36 "  testing broadcast attribute...\n"
$
$ %1 is the length of %2 which is what is bound, %3 is the port, %4 is the hostname
37 "    bound to \"%2$.*1$s[%3$lu]\", result=\"%4$s\"\n"
38 "  testing broadcast binding...\n"
$
$ %1 is the length of %2 which is what is bound, %3 is the port
39 "    bound to \"%2$.*1$s[%3$lu]\"\n"
40 "  \"Unknown interface\" exception correctly raised\n"
41 "*** Call that should have failed succeeded!\n"
$
$ NOTRANS
42 "y"
$
$ The next 17 messages are DEVELOP
$
43 "*** Can't register \"perfb\" interface - %%1$s\n"
44 "  result of forwarded call = \"%%1$s\"\n"
45 "  warning: server doesn't implement \"perfb_in\" procedure (probably an old server)\n"
46 "           large forwarded call test skipped\n"
$
$ %1 is the size we got, %2 is the size that we expected
47 "*** Sum mismatch in large forwarded call (%%1$lu, %%2$lu)\n"
48 "  large forwarded call completed OK\n"
49 "*** Can't unregister \"perfb\" interface - %%1$s\n"
50 "  Exception correctly raised - %%1$s\n"
51 "  WRONG exception raised - %%1$s\n"
52 "    ...in callback %%1$lu\n"
53 "    ...in idempotent callback %%1$lu\n"
54 "  pass %%1$2d...OK\n"
55 "*** op1 on Foo1 returned %%1$lu instead of 34\n"
56 "*** op2 on Bar2 returned %%1$lu instead of 15\n"
57 "*** op1 on Foo2 returned %%1$lu instead of 222\n"
58 "*** op2 on Bar1 returned %%1$lu instead of 65\n"
59 "  ...OK\n"
$
$ The next 3 messages are NOTRANS
$
60 "sleep"
61 "I/O"
62 "CPU"
$
$ The next 19 messages are DEVELOP.
$
$ %1 is the number of passes in this test, %2 is the number of calls per pass, 
$ %3 is a toggle on verification, %4 is a toggle on idempotent
63 "    passes: %%1$d; calls/pass: %%2$d; verification: %%3$s; idempotent: %%4$s"
64 "on"
65 "off"
66 "yes"
67 "no"
68 "; bytes/call: %%1$ld"
69 "; sleep secs: %%1$ld; mode: %%2$s"
$
$ %1 is the value that we got, %2 isvalue size that we expected
70 "*** Sum mismatch (%%1$lu, %%2$lu)\n"
71 "*** Output length (%%1$lu) is not equal to input length (%%2$lu)\n"
72 "*** Floating error #1 (%%1$g != %%2$g -- (%%3$g / %%4$g) * (%%5$g / %%6$g))\n"
73 "*** Floating error #2 (%%1$g != %%2$g -- (%%3$g / %%4$g) * (%%5$g / %%6$g))\n"
74 "*** Call count mismatch (%%1$lu, %%2$d)\n"
75 "        pass %%1$3d"
76 "; real/call: %%1$6.1f ms; calls/sec: %%2$5.1f"
77 "; kbytes/sec: %%1$5.1f"
78 "; real/call: %%1$4ld ms; calls/sec: %%2$3ld"
79 "; kbytes/sec: %%1$3ld"
80 "; elapsed time = 0 - try a larger 'calls/pass' value"
81 "  Multithreading: # tasks = %%1$d\n"
$
$ The next 8 messages are NOTRANS
$
82 "-"
83 "d"
84 "i"
85 "x"
86 "l"
87 "s"
88 "m"
89 "a"
$
$ USER
90 "%%1$s test [%%2$d]"
$
$ The next 5 messages are DEVELOP.
$
91 " (debug on)"
92 " (rpc__debug @ %%1$08x)"
93 " (async ack on)"
94 " (async ack off)"
95 " (lossy on)"
$
$ The next 2 messages are DEVELOP.
$
96 "*** Exception raised - %%1$s\n"
97 "\nStatistics:\n"
$ 
$ DEVELOP
98 "MaxCalls = 1 and not running in distinguished task!\n"
$
$ The next 7 messages are DEVELOP.
$
99 "*** Broadcast %%1$d\n"
100 "*** Maybe %%1$d\n"
101 "*** Broadcast maybe %%1$d\n"
102 "Can't use_family - %%1$s\n"
103 "Can't convert sockaddr to name - %%1$s\n"
$
$ %1 is the name of the socket, %2 is the port number of the socket
104 "Got socket: %%1$s[%%2$ld]\n"
105 "Can't RPC-register - %%1$s\n"
$
$ NOTRANS
106 "perfb"
$
$ The next 18 messages are DEVELOP.
$
107 "Can't LB-register - %%1$s\n"
108 "Can't LB-unregister - %%1$s\n"
109 "Can't RPC-unregister - %%1$s\n"
110 "    ...QUIT_ACTIVITY_FAULT caught\n"
111 "    ...signal caught - %%1$s\n"
112 "Sleeping for %%1$lu seconds...\n"
113 "    ...awake!\n"
114 "CPU looping for %%1$lu seconds...\n"
115 "    ...done!\n"
116 "0123456789ABCDE\n"
117 "/tmp/perfXXXXXX"
$
$ %1 is the file name, %2 is the size of the file
118 "Writing file \"%%1$s\" (size=%%2$d bytes)\n"
119 "Can't create temp file"
120 "Write failed"
121 "Read failed"
122 "    ...read %%1$d bytes (%%2$d)\n"
123 "Slow mode I/O net implemented on MS/DOS\n"
124 "    ...count mismatch [%%1$lu, %%2$u]\n"
$
$ The next 24 messages are NOTRANS
$
125 "The version of idl_base.h is not compatible with this stub/switch code."
126 "The version of rpc.idl is not compatible with this stub/switch code."
127 "The version of ncastat.idl is not compatible with this stub/switch code."
128 "The version of idl_base.h is not compatible with this stub/switch code."
129 "The version of rpc.idl is not compatible with this stub/switch code."
130 "The version of ncastat.idl is not compatible with this stub/switch code."
131 "The version of idl_base.h is not compatible with this stub/switch code."
132 "The version of rpc.idl is not compatible with this stub/switch code."
133 "The version of ncastat.idl is not compatible with this stub/switch code."
134 "The version of idl_base.h is not compatible with this stub/switch code."
135 "The version of rpc.idl is not compatible with this stub/switch code."
136 "The version of ncastat.idl is not compatible with this stub/switch code."
137 "The version of idl_base.h is not compatible with this stub/switch code."
138 "The version of rpc.idl is not compatible with this stub/switch code."
139 "The version of ncastat.idl is not compatible with this stub/switch code."
140 "The version of idl_base.h is not compatible with this stub/switch code."
141 "The version of rpc.idl is not compatible with this stub/switch code."
142 "The version of ncastat.idl is not compatible with this stub/switch code."
143 "The version of idl_base.h is not compatible with this stub/switch code."
144 "The version of rpc.idl is not compatible with this stub/switch code."
145 "The version of ncastat.idl is not compatible with this stub/switch code."
146 "The version of idl_base.h is not compatible with this stub/switch code."
147 "The version of rpc.idl is not compatible with this stub/switch code."
148 "The version of ncastat.idl is not compatible with this stub/switch code."
$
$ ------------------------------------- Server Tests --------------------------
$ 
$ These are the messages for the server part of the NCS test.
$ The next 11 messages are DEVELOP.
$
149 "Can't register - %%1$s\n"
150 "Can't register Foo mgr - %%1$s\n"
151 "Can't register Bar mgr - %%1$s\n"
152 "Can't register Foo1 object - %%1$s\n"
153 "Can't register Bar1 object - %%1$s\n"
154 "Can't register Bar2 object - %%1$s\n"
155 "Bad family name \"%%1$s\"\n"
156 "Can't use_family_wk - %%1$s\n"
157 "Can't convert sockaddr to name - %%1$s\n"
$
$ %1 is the socket name, %2 is the socket port
158 "Got socket: %%1$s[%%2$lu]\n"
$
$ The next 6 messages are NOTRANS.
$
159 "-"
160 "d"
161 "x"
162 "l"
163 "s"
164 "f"
$
$ The next 10 messages are DEVELOP.
$
165 "usage: server [-dxlsf] <max calls> <family name> ... \n"
166 "  -d: Turns on NCK runtime debug output\n"
167 "  -x: Turns on NCK runtime max debug output\n"
168 "  -l: Causes packets to be artificially lost\n"
169 "  -s: Turns on socket debug output (MS/DOS only)\n"
170 "  -f: Turns on fault mode (faults cause server exit)\n"
171 "*** Exception raised - %%1$s\n"
172 "Listening (rpc__debug @ %%1$08x)...\n"
173 "Listening...\n"
174 "Shutdown.\n"
$
$ The next 7 messages are NOTRANS.
$ 
175 "Heap is corrupted\n"
176 "\n"
$
$ %1 is an entry point, %2 is the size of the heap
177 "%%1$p %%2$5.5u  "
178 "Empty heap\n"
179 "%%1$lu bytes in use\n"
$
$ DEBUG
180 "Socket state:\n"
181 "\netext = %%1$08x, edata = %%2$08x, end = %%3$08x\n\n"
