$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)46        1.2  src/msg/bos/base/example.msg, bosmsg, msg411, 9431A411a 8/1/94 06:35:44
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
$ example.msg
$
$ Message source object file used by the gencat utility for message catalogue
$ generation.
$
$quote "
$delset 7
$set 7
$
$ ------------------------------------ Test Driver ----------------------------
$
$ The next 3 messages are NOTRANS.
1  "3ffa59166000.0d.00.01.4e.4f.00.00.00"
2  "%%1$s: "
3  "%%1$ld"
$
$ DEVELOP.
4  "Expected integer input, try again\n"
$
$ The next 2 messages are NOTRANS.
5  "%%1$*[^\n]"
6  "%%1$lx"
$
$ DEVELOP.
7  "Expected hex integer input, try again\n"
$
$ NOTRANS.
8  "%%1$*[\n]%%2$[^\n]"
$
$ The next 28 messages are DEVELOP.
$
9  "Expected string input, try again\n"
10  "              1 - request license\n"
11  "              2 - request_and_wait\n"
12  "              3 - wait_status\n"
13  "              4 - wait_remove\n"
14  "              5 - release license\n"
15  "              6 - cleanup \n"
16  "              7 - check license \n"
17  "              8 - request compound \n"
18  "              9 - current users \n"
19  "             10 - get all product info\n"
20  "             11 - get product info \n"
21  "             12 - get nodelock info \n"
22  "             13 - log message \n"
23  "             14 - add nodelock password \n"
24  "             15 - get target \n"
25  "             16 - set target id \n"
26  "             17 - request loop \n"
27  "             18 - quit \n\n"
28  "Any"
29  "Nodelocked"
30  "Concurrent"
31  "Compound"
32  "Use Once"
33  "Usage Metering"
34  "\n\nend of client test.\n\n"
35  "exiting... (status = %%1$lx)\n"
36  "\nclient test (2.0)\n\n"
$
$ NOTRANS
37  "-o"
$
$ DEVELOP.
38  "no file name provided for output\n"
$
$ NOTRANS.
39  "w"
$
$ DEVELOP.
40  "could not open %%1$s\n"
$
$ The next 2 messages are NOTRANS.
41  "-v"
42  "-z"
$
$ DEVELOP.
43  "enter the vendor_id (d for default vid = %%1$s)\n"
$
$ The next 2 messages are NOTRANS.
44  "%%1$s"
45  "d"
$
$ DEVELOP message.
46  "enter the vendor key (d for default key = 1234)\n"
$
$ NOTRANS.
47  "CLIENT_JOB_ID"
$
$ DEVELOP.
48  "uuid__decode of CLIENT_JOB_ID failed, using default"
$
$ NOTRANS.
49  "init"
$
$ DEVELOP.
50  "enter operation number"
$
$ NOTRANS.
51  "\n"
$
$ DEVELOP.
52  "not implemented\n\n"
$
$ NOTRANS.
53  "\n\n\n"
$
$ The next 7 messages are DEVELOP.
$
54  "enter nodelocked product password"
55  "enter license annotation in quotes"
56  "enter comment for nodelock file"
57  "add nodelock password"
58  "\nok.\n"
59  "enter trans id"
60  "enter check period"
$
$ NOTRANS.
61  "\n\n"
$
$ DEVELOP.
62  "not a legal id (0<=id<=%%1$ld)\n"
$
$ NOTRANS.
63  "check_license"
$
$ The next 15 messages are DEVELOP.
$
64  "ok.\n"
65  "    1. Nodelocked\n"
66  "    2. Concurrent\n"
67  "    3. Compound\n"
68  "    4. Use Once\n"
69  "    5. Usage Metering\n"
70  "enter type of license"
71  "enter product id"
72  "enter version"
73  "enter number of licenses"
74  "enter target id (in hex)"
75  "enter target type (a[ny], d[omain], h[pux], i[ntergraph], n[ext], sco, sgi, sun, svr4, u[ltrix], v[ms])"
76  "enter start date (seconds since 1970)"
77  "enter duration"
78  "illegal license type (must be between 1 and 5)\n"
$
$ The next 11 messages are NOTRANS.
$
79  "a"
80  "n"
81  "i"
82  "g"
83  "o"
84  "h"
85  "e"
86  "s"
87  "u"
88  "c"
89  "v"
$
$ The next 5 messages are DEVELOP.
$ 
90  "invalid target type\n"
91  "request_compound"
92  "vendor password: %%1$s\n"
93  "product password: %%1$s\n"
94  "license annotation: %%1$s\n"
$
$ NOTRANS
95  "    %%1$-10s %%2$-10s %%3$-10s %%4$-5s %%5$-30s\n"
$
$ The next 5 messages are DEVELOP.
$ 
96  "user"
97  "node"
98  "group"
99  "amt"
100  "start time"
$
$ The next 3 messages are NOTRANS.
$
101  "get_cur_users"
102  "\0"
$
$       Format string for user, node, group, number of licenses, and time/date
103  "    %%1$-10s %%2$-10s %%3$-10s %%4$-5ld %%5$-30s\n"
$
$ DEVELOP.
104  "\ncount: %%1$ld\n"
$
$ The next 2 messages are NOTRANS.
105  "get product info"
106  "get all product info"
$
$ The next 28 messages are DEVELOP.
$
107  "product id: %%1$ld\n"
108  "product name: %%1$s\n"
109  "product version: %%1$s\n"
110  "installed licenses: %%1$ld\n"
111  "in_use: %%1$ld\n"
$
$       Year, month, day
112  "start date: %%s\n"
$
$       Year, month, day
113  "expiration date: %%s\n"
114  "time stamp: %%1$ld\n"
115  "license annotation: %%1$s\n\n"
116  "Domain"
117  "Sun"
118  "VAX/VMS"
119  "MS-DOS"
120  "HP-UX"
121  "Ultrix"
122  "HP-OSF"
123  "Svr4"
124  "Intergraph"
125  "SCO-UNIX"
126  "SGI"
127  "NeXT"
128  "AIX"
129  "Novell"
130  "DG/UX"
131  "target type  : %%1$s\n"
132  "target id    : %%1$lx\n"
133  "enter message"
134  "not a legal id (-1<=id<=%%1$ld)\n"
$
$ The next 4 messages are NOTRANS.
$
135  "log_msg"
136  "get nodelock info"
$
$       Error message followed by an id
137  "%%1$s (id = %%2$ld)\n"
138  "%%1$s \n"
$
$ DEVELOP.
139  "not a legal trans id (0<=id<=%%1$ld)\n"
$
$ NOTRANS.
140  "release"
$
$ The next 4 messages are DEVELOP.
$
141  "    0. Any Type\n"
142  "    3. Use Once\n"
143  "enter number of licenses to request"
144  "illegal license type (must be between 0 and 3)\n"
$
$ NOTRANS.
145  "req_and_wait"
$
$ The next 4 messages are DEVELOP.
$
146  "position in queue %%1$ld = %%2$ld\n"
147  "\ntrans id = %%1$ld\n"
148  "license granted - trans id = %%1$ld\n"
149  "license type = %%1$s\n"
$
$ NOTRANS.
150  "request"
$
$ DEVELOP.
151  "license annotation: %%1$s \n"
$
$ The next 2 positions are NOTRANS.
152  "wait_remove"
153  "wait_status"
$
$ The next 13 messages are DEVELOP.
$
154  "position in queue %%1$d = %%2$ld\n"
155  "    4. Nodelock/Concurrent\n"
156  "    5. Concurrent/No Release\n"
157  "    6. Concurrent/Request_and_wait\n"
158  "enter number of iterations"
159  "illegal license type (must be between 0 and 6)\n"
160  "nodelock request"
161  "nodelock license granted - index = %%1$ld\n"
162  "nodelock release"
163  "nodelock released ok; break=0x%%1$lx.\n"
164  "license granted - index = %%1$ld\n"
165  "exiting without cleanup.\n"
166  "waiting for check period to expire"
$
$ NOTRANS.
167  ".\n"
$
$ DEVELOP.
168  "released ok; break=0x%%1$lx.\n"
$
$ --------------------------------- Example of Calls to NetLS ---------------------------
$
$ The next 3 messages are NOTRANS.
169  "376e649e1000.0d.00.00.d6.44.00.00.00"
170  "ls_ex"
171  "1.0"
$
$ The next 3 messages are DEVELOP.
$
172  "new"
173  "\nQuitting ...\n"
174  "Quit"
$
$ NOTRANS.
175  "Init"
$
$ DEVELOP.
176  "\nLicense granted.\n"
$
$ NOTRANS.
177  "Request"
$
$ DEVELOP.
178  "\nDo you want to wait for a license? "
$
$ The next 3 messages are NOTRANS.
$
179  "%%1$c"
180  "n"
$
$ Format string for next two messages
181  "\n%%1$s %%2$s"
$
$ The next 2 messages are DEVELOP.
182  "Please enter time (in seconds, maximum 100)"
183  "between queries to server: "
$
$ The next 3 messages are NOTRANS.
184  "%%1$d"
185  "Request and wait"
$
$ Format string for next two messages
186  "\n%%1$s %%2$s\n"
$
$ The next 3 messages are DEVELOP.
187  "Your positions"
188  "on the server queues are:\n"
189  "        Queue     Position\n"
$
$ NOTRANS.
190  "        %%1$5d     %%2$8d\n"
$
$ The next 3 messages are DEVELOP.
191  "Current users"
192  "\nThe current users of "
193  "this product are:\n"
$
$	Format string for "User", "Node", "Group", "Number", "Start Time". 
$ NOTRANS.
194  "\n%%1$10s %%2$10s %%3$11s %%4$12s %%5$17s\n"
$
$ The next 5 messages are DEVELOP.
$
195  "User"
196  "Node"
197  "Group"
198  "Number"
199  "Start Time"
$
$	User_id, Node name, Group id, number of licenses, time started. NOTRANS
200  "%%1$10s %%2$10s %%3$11s %%4$12d %%5$32s"
$
$ DEVELOP.
201  "\nDo you want to continue waiting? "
$
$ The next 2 messages are NOTRANS.
202  "Wait status"
203  "Wait remove"
$
$ The next 2 messages are DEVELOP.
204  "\nStarting up application.\n"
205  "This is the application ls_ex."
$
$ NOTRANS.
206  "Log message"
207  "Check"
$
$ DEVELOP.
208  "\nEnding application.\n"
$
$ NOTRANS.
209  "Release"
$
$ The next 5 messages are DEVELOP.
$
210  "\nLicense released.\n"
211  "Calc request"
212  "\nObtained calculation license.\n"
213  "Calc release"
214  "\nReleased calculation license.\n"
$
$ NOTRANS.
215  "%%1$s"
$
$ DEVELOP.
216  "error"
$
$ --------------------------- Short Example of Calls to NetLS ---------------------------
$
$ The next 5 messages are NOTRANS.
$
217  "382d89dd8000.0d.00.00.80.9c.00.00.00"
218  "1.0"
219  "init"
220  "%%1$s \n"
221  "req_and_wait"
$
$ DEVELOP.
222  "not waiting on any queues: %%1$s \n"
$ 
$ The next 2 messages are NOTRANS.
223  "wait_stat"
224  "release"
$
$ DEVELOP.
225  "error"
$
$ More Test Drivers messages.  They are DEVELOP.
$
226  "OSF/1"
227  "OS/2"
228  "OS/400"
229  "MVS"
230  "Win/NT"
231  "p"
232  "m"
233  "f"
