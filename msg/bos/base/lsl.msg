$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)50        1.6  src/msg/bos/base/lsl.msg, bosmsg, msg411, 9438B411a 9/23/94 10:06:37
$
$   COMPONENT_NAME: libncs
$
$   FUNCTIONS:
$
$   ORIGINS: 84
$
$
$ lsl.msg
$
$ Message source object file used by the gencat utility for message catalogue
$ generation.
$
$quote "
$delset 1
$set 1
$
$ --------------------------------- End-user licenses -------------------------
$
$ ADMIN message.
1  "ls_dpass"
$
$ NOTRANS.
2  "#"
$
$ The next 3 messages are ADMIN.
3  "-usage"
4  "-h"
5  "-version"
$
$ The next 3 messages are NOTRANS.
$
6  "\t"
7  "\\"
8  "?(ls_dpass)  %%1$s\n"
$
$ The next 10 messages are ADMIN.
$
9  "LS_DPASS Version %%1$s\n"
10  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
11  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
12  "Could not open or create %%1$s"
13  "Don't know how to convert version 3 of product database"
14  "Wrong version of product database"
15  "product file corrupted - current record index is wrong"
16  "product file corrupted - current vendor index is wrong"
17  "?(ls_dpass)  Current product index is invalid - please set product\n"
18  "Could not open or create %%1$s\n-- restore from %%2$s"
$
$ The next 4 messages are NOTRANS.
19  "\0"
20  " "
21  "\n"
$
$ The next 30 messages are ADMIN.
$
22  "-f"
23  "Could not open batch input file"
24  "\nCommand line usage:  ls_dpass\n"
25  "    [-a annotation]\n"
26  "    [-d duration] (use -d or -e)\n"
27  "    [-D Aggregate duration] ( compound only )\n"
28  "    [-e expiration] (yy/mm/dd)\n"
29  "    [-E derived expiration] (yy/mm/dd)\n"
30  "    [-h ] (help)\n"
31  "    [-i vendor id]\n"
32  "    [-k key]\n"
33  "    [-l license type] (c[oncurrent], n[odelocked], u[seonce], or m[etered])\n"
34  "    [-l license type] (c[oncurrent], n[odelocked], or u[seonce])\n"
35  "    [-m multi-usage specification] (u[ser], n[ode], g[roup], j[ob])\n"
36  "    [-n number of licenses]\n"
37  "    [-N product name]\n"
38  "    [-p product id]\n"
39  "    [-r revision] (version)\n"
40  "    [-s start] (yy/mm/dd)\n"
41  "    [-S derived start] (yy/mm/dd)\n"
42  "    [-t target type] (any, aix, as[/400], dg[ux], do[main], h[pux],\n"
43  "                i[ntergraph], ms[dos], mv[s], ne[xt], no[vell],\n"
44  "                sgi, sun, svr4, u[ltrix], v[ms], w[in/nt], default is domain)\n"
45  "    [-u] (create script for ls_admin or nodelock file)\n"
46  "    [-usage] (produce this list)\n"
47  "    [-v vendor name]\n"
48  "    [-version] (version of this tool)\n"
49  "    [-w password type] (l[icense] or c[ompound])\n"
50  "    [-z] (debugging trace output)\n"
51  "    target(s) \n"
$
$ The next 3 messages are NOTRANS.
$
52  "\0"
53  "c"
54  "l"
$
$ The next 5 messages are ADMIN.
$
55  "a:d:D:e:E:i:k:l:m:n:N:p:r:s:S:t:uPv:w:z"
56  "a"
57  "License Annotation must be 80 characters or less in length"
58  "License Annotation not found"
59  "d"
$
$ NOTRANS.
60  "%%1$d"
$
$ The next 17 messages are ADMIN.
$
61  "D"
62  "e"
63  "E"
64  "i"
65  "create"
66  "k"
67  "m"
68  "u"
69  "g"
70  "n"
71  "j"
72  "-m requires [u,g,n,j]"
73  "Number of licenses argument missing"
74  "Number of licenses cannot be negative or zero"
75  "N"
76  "P"
77  "p"
$
$ NOTRANS.
78  "%%1$ld"
$
$ The next 24 messages are ADMIN.
$
79  "Product ID cannot be less than zero"
80  "r"
81  "s"
82  "S"
83  "t"
84  "o"
85  "h"
86  "v"
87  "w"
88  "z"
89  "No targets given"
90  "Negative value for number of targets is illegal"
91  "Number of licenses specified must be >= 1"
92  "-D, -E, and -S are only valid with compound password type"
93  "Vendor not found - please specify vendor id with -i option"
94  "Vendor not found - please specify vendor key with -k option"
95  "The specified vendor id (%%1$s)\ndoes not match the vendor id (%%2$s) found in\nthe product data base for the vendor %%3$s."
96  "The specified vendor key does not match the vendor key found in\nthe product data base for the vendor %%1$s."
97  "No vendors in database - please specify vendor id, name, and key with -i, -v, and -k option"
98  "product %%1$ld"
99  "Specify product id with -p option"
100  "Invalid password type"
101  "multi-usage parameter (-m) only works with Concurrent license type"
102  "Target ids are not applicable when target type is any"
$
$ The next 6 messages are NOTRANS.
$
103  "C"
104  "8"
105  "4"
$
$	Format for an internet address
106  "%%1$2X%%2$1X%%3$2X%%4$1X%%5$2X%%6$1X%%7$2X"
$
$	inet address, message
107  "%%1$s: %%2$s"
108  "%%1$x"
$
$ The next 8 messages are ADMIN.
$
109  "Number of licenses must equal number of targets for nodelocked and usage-metered passwords"
110  "Number of licenses must be >= to number of servers specified"
111  "******************"
112  "*servers*"
113  "Please Select a Vendor"
114  "Please Select a Product"
115  "Current Customer: "
116  "No Customer Information entered"
$
$ The next 3 messages are NOTRANS.
$
$ Format for message output, %2 is the object affected by the event mentioned
$ by %1. 
117  "%%1$s%%2$s"
$
$ %2 is the object affected by the event mentioned by %1. %3 is the version.
118  "%%1$s%%2$s [%%3$s]"
119  "   Count: %%1$d"
$
$ The next 4 messages are ADMIN.
$
$  %1 is the current target. %2 is the current total.
120  "Target %%1$d of %%2$d"
121  "Target 1 of 1    "
122  "\nDialog interface usage:  ls_dpass\n"
123  "?(ls_dpass)  error: %%1$i invalid argument(s) preceeding graphic argument delimiter \"%%2$s\" :\n"
$
$ The next 15 messages are NOTRANS.
$
124  ">>    %%1$s\n"
125  "%%1$s\n"
126  "\0"
127  "?(ls_dpass) %%1$s\n"
128  "\n\n\n"
129  "%%1$x"
130  "c"
131  "C"
132  "8"
133  "4"
$
$ Format sting for an internet address
134  "%%1$2X%%2$1X%%3$2X%%4$1X%%5$2X%%6$1X%%7$2X"
$
$ Format string for "A product with this id, but a different name, exists",
$ (product name)
135  "%%1$s (%%2$s)"
$ Format string for "A product with this name, but different id, exists".
$ (product name)
136  "%%1$s (%%2$d)"
137  "r"
138  "a"
$
$ The next 3 messages are ADMIN.
$
139  "\n--------------------- N L S P A S S ---------------------\n\n"
140  "Customer name:  %%1$s\n"
141  "      Address:  %%1$s\n"
$
$ NOTRANS.
142  "                %%1$s\n"
$
$ ADMIN message.
143  "      Contact:  %%1$s\n"
$
$ The next 3 messages are NOTRANS.
$
144  "Apollo"
145  "%%1$s [%%2$s]"
146  "\n"
$
$ The next 24 messages are ADMIN.
$
147  "Domain"
148  "Any"
149  "Sun"
150  "VAX/VMS"
151  "MS-DOS"
152  "HP-UX"
153  "Ultrix"
154  "HP-OSF"
155  "SVR4"
156  "Intergraph"
157  "SCO_UNIX"
158  "SGI"
159  "NeXT"
160  "AIX"
161  "Novell"
162  "DG-AViiON"
163  "Concurrent Access"
164  "Node Locked"
165  "Use Once"
166  "Compound"
167  "Usage Metering"
168  "?(ls_dpass) %%1$s\n"
169  "Vendor: %%1$s,  Vendor id: %%2$s,  Vendor key: %%3$s\n"
170  "Product: %%1$s, Product id: %%2$ld\n"
$
$ Format string for name and version. NOTRANS.
171  "%%1$s [%%2$s]"
$
$ The next 14 messages are ADMIN.
$
172  "\nVendor Password %%1$s \n"
173  "built as\n"
174  "   vendor name : %%1$s\n"
175  "   vendor id   : %%1$s\n"
176  "   target      : any \n"
177  "   target      : %%1$x \n"
178  "\nProduct Passwords built as\n"
179  "   name         : %%1$s\n"
180  "   id           : %%1$ld\n"
181  "   version      : %%1$s\n"
182  "   password type: %%1$s\n"
183  "   license type : %%1$s\n"
184  "\n    Product password for "
185  "any"
$
$ The next 4 messages are NOTRANS.
$
186  "%%1$s:%%2$s"
187  "%%1$x"
188  "0x080009%%1$06x"
189  "0x%%1$x"
$
$ The next 12 messages are ADMIN.
$
190  " server : %%1$s\n"
191  "    built as\n"
192  "       license annotation : %%1$s\n"
193  "       start date         : "
194  "       duration           : %%1$d\n"
195  "       expiration         : "
196  "       number             : %%1$d\n"
197  "       multi usage rules  : Same"
198  " user"
199  " group"
200  " node"
201  " job"
$
$ NOTRANS.
202  ".\n"
$
$ The next 9 messages are ADMIN.
$
203  "       aggregate duration : %%1$d\n"
204  "       derived start      : "
205  "       derived expiration : "
206  "       license type       : %%1$s\n"
207  "       target type        : %%1$s\n"
208  "       target             : Any\n"
$
$ Model, serial and server id
209  "       target             : %%1$s:%%2$s  (0x%%3$x)\n"
210  "       target             : %%1$x\n"
211  "       target             : 0x080009%%1$06x\n"
$
$ Vendor name, vendor id, password. NOTRANS.
212  "ls_admin -a -v \"%%1$s\" %%2$s %%3$s\n\n\n"
$
$ ADMIN message.
213  "\nPut the following line into your nodelock file: \n\n"
$
$ The next 7 messages are NOTRANS.
$
$ Vendor id, password, and version
214  "%%1$s %%2$s \"\" \"%%3$s\"\n\n\n"
$
$ Vendor id, password, liocense annotation, and version
215  "%%1$s %%2$s \"%%3$s\" \"%%4$s\"\n\n\n"
$
$ Vendor name, product name, password, and version
216  "ls_admin -a -p \"%%1$s\" \"%%2$s\" %%3$s \"%%4$s\"\n"
$
$ License annotation, vendor name, product name, password, and version
217  "ls_admin -a -l \"%%1$s\" -p \"%%2$s\" \"%%3$s\" %%4$s \"%%5$s\"\n"
218  "?(ls_dpass) Warning: One or more servers were unable to be contacted.\n"
219  "                    iFOR/LS License count may be wrong.\n"
220  "\nYou now have %%1$d iFOR/LS use-once licenses available.\n\n"
$
$ The next 12 messages are ADMIN.
$
221  "LS_DPASS"
222  "XLs_DPass"
223  "Exit"
224  "Help Index..."
225  "Done"
226  "field"
227  "info"
228  "Add Vendor..."
229  "Add Product..."
230  "Show Vendor"
231  "Show Product"
232  "Delete..."
$
$ The next 4 messages are NOTRANS.
233  "iFOR/LS License Type:"
234  "Use-Once"
235  "Node Locked"
236  "Developer"
$
$ The next 11 messages are ADMIN.
$
237  "Operations:"
238  "Operate On:"
239  "Vendor"
240  "Product"
241  "Password"
242  "Customer"
243  "ADD VENDOR"
244  "Vendor Name"
245  "Vendor Id"
246  "Vendor Key"
247  "Create Vendor Id"
$
$ NOTRANS.
248  "Vendor Password"
$
$ The next 5 messages are ADMIN.
249  "ADD PRODUCT"
250  "Product Name"
251  "Product Id"
252  "Product Version"
253  "License Annotation"
$
$ NOTRANS.
254  "License Password"
$
$ The next 40 messages are ADMIN.
$
255  "Customer Name"
256  "Address"
257  "Contact"
258  "Target Id"
259  "Target Type: "
260  "Next Target"
261  "Start"
262  "Start Date"
263  "Derived Start"
264  "Derived Start Date"
265  "Duration"
266  "Duration (days)"
267  "Derived Duration"
268  "Derived Duration (days)"
269  "Expiration"
270  "Expiration Date"
271  "Derived Expiration"
272  "Derived Expiration Date"
273  "Aggregate Duration"
274  "Aggregate Duration (days)"
275  "Number of licenses (this target)"
276  "Number of licenses"
277  "Number of licenses (total)"
278  "Create Script"
279  "Output file"
280  "Output file Name"
281  "Create passwords"
282  "Number of Targets"
283  "Multi-use rules:"
284  "Same User"
285  "Same Group"
286  "Same Node"
287  "Same Job"
288  "No Multi-use selection -- using supplier's compound password"
289  "No Annotation -- using supplier's compound password"
290  "CONFIRM"
291  "Delete"
292  "Delete Vendor?"
293  "Delete Product?"
294  "?(ls_dpass)  warning: %%1$i invalid argument(s) following graphic argument delimiter \"%%2$s\" :\n"
$
$ NOTRANS.
295  ">>    %%1$s\n"
$
$ The next 28 messages are ADMIN.
$
296  "or  ls_dpass %%1$s [ standard X arguments ]\n"
297  "s_v_add"
298  "s_p_add"
299  "Domain"
300  "Hp-ux"
301  "SunOS"
302  "Intergraph"
303  "NeXT"
304  "SGI"
305  "SVR4"
306  "SCO_UNIX"
307  "MS-DOS"
308  "Ultrix"
309  "VAX/VMS"
310  "AIX"
311  "Novell"
312  "DG/UX"
313  "Any"
314  "License"
315  "Compound"
316  "Concurrent"
317  "Use Once"
318  "Usage Meter"
319  "Password Type:"
320  "License Type:"
321  "Vendors"
322  "Products"
323  "Customers"
$
$ The next 2 are NOTRANS.
$
324  "             "
325  "                 "
$
$ The next 2 are ADMIN.
326  "FATAL ERROR: ifor_ls_encode called in nls_lib!!!\n"
327  "FATAL ERROR: ifor_ls_decode called in nls_lib!!!\n"
$
$ -------------------------------------- Get HP ID  Messages -----------------
$
$ The next 50 messages are NOTRANS.
$
328  "@(#) $Revision: 1.1 $ \n"
329  "\0"
330  "%%1$02x"
331  "/"
332  "???"
333  "840"
334  "0"
335  "\020"
336  "1"
$
$       HP node-id model number and model number letter for model number 1.
337  "%%1$5.5u-6990%%2$c"
$
$       HP node-id model number and model number letter.
338  "%%1$5.5u%%2$c"
$
$  HP node-id serial date, country and serial index.
339  "%%1$4.4u%%2$c%%3$5.5u"
$
$       Format for scanf for the first part and second part of the HP node-id.
340  "%%1$5s %%2$x"
341  "SPU"
342  "000000000000"
343  "LLA"
344  "9000/7"
345  " "
346  "standalone"
347  "+"
348  "_isc_table"
349  "stderr, nlist: failed\n"
350  "open"
351  "lseek"
352  "read"
353  "%%1$x"
$
$       ------------------------------- Get SVR4 ID Messages ------------------------------
$
354  "%%1$d"
355  "\0"
356  "/"
$
$ %1 is "/hinv/enet", %2 is the device name, %3 is the extension
357  "%%1$s/%%2$s/%%3$s"
358  ".attr"
359  "r"
360  "\n"
361  "net_addr"
362  "0"
363  "%%1$x"
364  ":"
365  "dlpi_req putmsg"
366  "dlpi_req getmsg"
$
$       Error message format string for failed reply from a sent request to the dlpi device
$       when querying the SVR4 node id.
367  "dlpi_req prim=%%1$x/%%2$x errno=%%3$d unix_errno=%%4$d\n"
$
$       ----------------------------- Get Ultrix ID Messages -----------------------------
$
368  "get_my_ether_addr: socket"
369  "get_my_ether_addr: ioctl (SIOCGIFCONF)"
370  "ioctl (SIOCGIFFLAGS)"
371  "ioctl (SIOCRPHYSADDR)"
$
$       --------------------------- Get Utility Option Messages ---------------------------
$
372  "="
373  "\0"
374  "/"
375  ";"
376  "]"
377  ":"
$
$ The next 3 messages are ADMIN.
378  "nextopt() error: no known options\n"
379  "nextopt() error: no arguments to parse\n"
380  "use \""
$
$ NOTRANS.
381  "%%1$s "
$
$ ADMIN.
382  "-usage\" to get usage\n"
$
$ The next 5 messages are NOTRANS.
$
383  "0"
384  "9"
385  "+"
386  "-"
387  "?"
$
$ %1 is the program name, %2 is what is illegal. ADMIN..
388  "%%1$s: illegal option: %%2$c\n"
$
$ %1 is the program name, %2 is what is illegal. ADMIN.
389  "%%1$s: illegal option -- %%2$c\n"
$
$ NOTRANS.
390  "["
$
$ The next 5 messages are ADMIN.
$
391  "usage"
392  "version"
393  "No known version"
$
$ %1 is the program name, %2 is what is unknown
394  "%%1$s: unknown option: %%2$s\n"
395  "option %%1$s ambiguous: matches"
$
$ The next 2 messages are NOTRANS. 
396  "\n"
397  " ="
$
$ The next 2 messages are ADMIN.
398  "<geometry-spec>"
399  "option requires an argument --"
$
$ NOTRANS.
400  "#"
$
$ The next 2 messages are ADMIN.
401  "option"
402  " expected integer argument\n"
$
$ The next 6 messages are NOTRANS.
403  ">"
404  "w"
405  "a"
406  "<"
407  "r"
408  ": "
$
$ ADMIN.
409  "Unknown code in options: %%1$c\n"
$
$ NOTRANS.
410  " %%1$c"
$
$ The next 3 messages are DEVELOP.
411  "The version of idl_base.h is not compatible with this stub/switch code."
412  "The version of rpc.idl is not compatible with this stub/switch code."
413  "The version of ncastat.idl is not compatible with this stub/switch code."
$
$ --------------------------------- Library for calls to iFOR/LS ----------------
$
$ ADMIN.
414  "cleanup handler"
$
$ DEVELOP.
415  "entering: %%1$s\n"
416  "%%1$s\n"
$
$	%1 is the length of time and %2 is the routine name. ADMIN.
417  "Took %%1$f in routine %%2$s\n"
$
$ The next 9 messages are DEVELOP.
$
418  "netls_create_job_id()"
419  "netls_init cleanup handler"
420  "netls_init"
421  "netls_request_compound()"
422  "Request compound cleanup handler"
423  "comm failure: invalidating svr"
424  "using short timeout"
425  "old svr: invalidating svr"
426  "turning short timeout off"
$
$ USER.
427  "error"
$
$ DEVELOP.
428  "adding entries to cache..."
$
$ USER.
429  "cache not refreshed"
$
$ The next four messages are DEVELOP.
430  "trying again with refreshed cache"
431  "Request_compound"
432  "netls_request_license_int()"
433  "Request cleanup handler"
$
$ NOTRANS.
434  "\0"
$ 
$ The next 34 messages are DEVELOP.
$
435  "using node locked license"
436  "Type is nodelocked, and nodelocked license not found"
437  "Multi-use check at server"
438  "Regular request to server"
439  "waiting in queue"
440  "Wait remove from server"
441  "netls_wait_stat()"
442  "comm failure: invalidating entry"
443  "wrong boot time: trying again"
444  "wait status at server"
445  "no longer in queue"
446  "netls_wait_remove()"
447  "regular wait remove"
448  "netls_release_license()"
449  "license is not concurrent"
450  "refreshing cache"
451  "rebound to server: trying again"
452  "release license"
453  "netls_cleanup()"
454  "license was not concurrent"
455  "releasing held license..."
456  "cleanup"
457  "license released"
458  "license not released"
459  "removing user from wait queue..."
460  "cleanup wait remove"
461  "user released from queue"
462  "user not released from queue"
463  "netls_check_license()"
464  "license is nodelocked - checking id pod"
465  "refreshing cache..."
466  "Check license"
467  "entering: %%1$s\n"
468  "%%1$s\n"
$
$	%1 is the length of time and %2 is the routine name. ADMIN.
469  "Took %%1$f in routine %%2$s\n"
$
$ DEVELOP.
470  "netls_get_cur_users()"
$
$ NOTRANS.
471  "\0"
$
$ The next 8 messages are DEVELOP.
$
472  "comm failure: invalidating server"
473  "using short timeout"
474  "first time"
475  "next time"
476  "trying new server..."
477  "calling server..."
478  "get current users"
479  "turing short timeout off"
$
$ USER 
480  "error"
$
$ The next 9 messages are DEVELOP.
$
481  "finished with current server"
482  "output is full"
483  "no more servers to visit"
484  "more servers to visit"
485  "netls_get_all_product_info()"
486  "netls_get_product_info()"
487  "netls_log_msg()"
488  "log message"
489  "netls_add_nodelocked()"
$
$ The next 3 messages are NOTRANS.
490  "\n"
491  "# "
492  " "
$
$ The next 10 messages are DEVELOP.
$
493  "add nodelock"
494  "comm failure: invalidating svr"
495  "old svr: invalidating svr"
496  "get active servers"
497  "turning short timeout off"
498  "The version of idl_base.h is not compatible with this stub/switch code."
499  "The version of rpc.idl is not compatible with this stub/switch code."
500  "The version of ncastat.idl is not compatible with this stub/switch code."
$
$	------------------------------ Subroutines for iFOR/LS library calls -----------------
$
501  "entering: %%1$s\n"
502  "%%1$s\n"
$
$	%1 is the length of time and %2 is the routine name. ADMIN.
503  "Took %%1$f in routine %%2$s\n"
$
$ USER.
504  "malloc failed - out of memory"
$
$ The next 7 messages are DEVELOP.
$
505  "comm failure: invalidating svr"
506  "using short timeout"
507  "old svr: invalidating svr"
508  "failure: returning to caller"
509  "get all products2"
510  "get products2"
511  "turning short timeout off"
$
$ The next 9 messages are NOTRANS.
$
512  "\0"
513  "`node_data"
514  "."
515  "/"
516  "A"
517  "Z"
518  "a"
519  " "
$
$	Process name and the accompanying error message
520  "%%1$s: %%2$s"
$
$ ADMIN.
521  "unknown"
$
$ NOTRANS.
522  ":"
$
$ The next 3 messages are DEVELOP.
523  "netls_init()"
524  "nls_init_lbcm()"
525  "Server count passed in was too high\n"
$
$ The next 4 messages are NOTRANS.
$
$ --------------------------------- I18N message routine ---------------------
$
526  "%%1$s"
$
$ Filename and message
527  "%%1$s: %%2$s"
$
$ ------------------------------- Test nodelock passwords ---------------------
$
528  "3ffa59166000.0d.00.01.4e.4f.00.00.00"
529  "1"
$
$ DEVELOP.
530  "usage: nl_test [-i nodeid] password [annot] [version]\n"
$
$ The next 6 messages are NOTRANS.
531  "-"
532  "v"
533  "k"
534  "%%1$d"
535  "i"
536  "%%1$x"
$
$ The next 9 messages are DEVELOP.
$
537  "password ok"
538  "license has expired"
539  "license has not started"
540  "version mismatch"
541  "annotation mismatch"
542  "wrong target"
543  "bad password or vendor incorrect"
544  "bad password version"
545  "status=0x%%1$x\n"
$ 
$ NOTRANS.
546  "%%1$s\n"
$
$ The next 21 messages are DEVELOP.
$
547  "Password Version #%%1$d\n"
$
$	Target id and target type
548  "Target ID:\t%%1$x (%%2$s)\n"
549  "Product ID:\t%%1$d\n"
$
$	License timestamp and the date that translates to
550  "Time Stamp:\t%%1$d = %%2$s"
551  "Start date:\t"
552  "Exp date:\t"
553  "Domain"
554  "Any"
555  "Sun"
556  "VAX/VMS"
557  "MS-DOS"
558  "HP-UX"
559  "Ultrix"
560  "HP/OSF"
561  "SVR4"
562  "Intergraph"
563  "SCO_UNIX"
564  "SGI"
565  "NeXT"
566  "AIX"
567  "Unknown(%%1$d)"
$
$ ------------------------------ Verify NLS_PASS installation -----------------
$
$ The next 16 messages are ADMIN.
$
568  "=usage"
569  " =usage option not valid for ls_pass_tv\n\n"
570  "-usage"
571  "-h"
572  "-version"
573  "-v"
574  "?(ls_pass_tv) no arguments required.\n"
575  "init"
576  "A developers license is not present on this node."
577  "A developer's license is present on this node."
578  "You have %%1$d iFOR/LS use-once licenses installed."
579  "iFOR/LS licenses not present."
580  "A iFOR/LS node locked license is present on this node."
581  "Active iFOR/LS Servers:"
582  "No servers found"
$
$	Server name, OS version, iFOR/LS version
583  "    %%1$s (%%2$s) running iFOR/LS %%3$s\n"
$
$ The next 2 messages are NOTRANS.
584  "?(ls_tv) %%1$s\n"
585  "%%1$s\n"
$ 
$ The next 4 messages are ADMIN.
$
586  "usage: ls_pass_tv [-h] [-usage] [-version]\n"
587  "LS_PASS_TV Version %%1$s-- LS_PASS Test and Verification Tool\n"
588  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
589  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
$
$ ----------------------------------- LS_PASS ---------------------------------
$
$ The next 2 messages are NOTRANS.
590  "ls_pass"
591  "#"
$
$ The next 6 messages are ADMIN.
$
592  "-usage"
593  "-h"
594  "-version"
595  "You need a nodelocked demo license to use this tool\n"
596  "If you are confused, contact the iFOR/LS marketing product manager\n"
597  "-f"
$
$ NOTRANS.
598  "r"
$
$ ADMIN.
599  "Could not open batch input file"
$
$ NOTRANS.
600  "?(ls_pass)  %%1$s\n"
$
$ The next 3 messages are ADMIN.
601  "LS_PASS Version %%1$s\n"
602  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
603  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
$
$ NOTRANS.
604  "%%1$d"
$
$ The next 7 messages are ADMIN.
$
605  "Could not open or create %%1$s"
606  "Don't know how to convert version 3 of product database"
607  "Wrong version of product database"
608  "product file corrupted - current record index is wrong"
609  "product file corrupted - current vendor index is wrong"
610  "?(ls_pass)  Current product index is invalid - please set product\n"
611  "Could not open or create %%1$s\n-- restore from %%2$s"
$
$ The next 4 messages are NOTRANS.
612  "\0"
613  " "
614  "\t"
615  "\n"
$
$ The next 30 messages are ADMIN.
$
616  "\nCommand line usage:  ls_pass\n"
617  "    [-a annotation]\n"
618  "    [-d duration] (use -d or -e)\n"
619  "    [-D Aggregate duration] ( compound only )\n"
620  "    [-e expiration] (yy/mm/dd)\n"
621  "    [-E derived expiration] (yy/mm/dd)\n"
622  "    [-f batch file] (containing full ls_pass command lines)\n"
623  "    [-h ] (help)\n"
624  "    [-i vendor id]\n"
625  "    [-k key]\n"
626  "    [-l license type] (c[oncurrent], n[odelocked], u[seonce], or m[etered])\n"
627  "    [-l license type] (c[oncurrent], n[odelocked], or u[seonce])\n"
628  "    [-m multi-usage specification] (u[ser], n[ode], g[roup], j[ob])\n"
629  "    [-n number of licenses]\n"
630  "    [-N product name]\n"
631  "    [-p product id]\n"
632  "    [-P] (product id field in password limited to 16-bits)\n"
633  "    [-r revision] (version)\n"
634  "    [-s start] (yy/mm/dd)\n"
635  "    [-S derived start] (yy/mm/dd)\n"
636  "    [-t target type] (an[y], ai[x], as[/400], dg[ux], do[main], h[pux],\n"
637  "                i[ntergraph], ms[dos], mv[s], ne[xt], no[vell],\n"
638  "                sgi, sun, svr4, u[ltrix], v[ms], w[in/NT], default is domain)\n"
639  "    [-u] (create script for ls_admin or nodelock file)\n"
640  "    [-usage] (produce this list)\n"
641  "    [-v vendor name]\n"
642  "    [-version] (version of this tool)\n"
643  "    [-w password type] (l[icense] or c[ompound])\n"
644  "    [-z] (debugging trace output)\n"
645  "    target(s) \n"
$
$ The next 3 messages are NOTRANS.
$
646  "\0"
647  "c"
648  "l"
$
$ The next 5 messages are ADMIN.
$
649  "a:d:D:e:E:i:k:l:m:n:N:p:r:s:S:t:uPv:w:z"
650  "a"
651  "License Annotation must be 80 characters or less in length"
652  "License Annotation not found"
653  "d"
$
$ NOTRANS.
654  "%%1$d"
$
$ The next 17 messages are ADMIN.
$
655  "D"
656  "e"
657  "E"
658  "i"
659  "create"
660  "k"
661  "m"
662  "u"
663  "g"
664  "n"
665  "j"
666  "-m requires [u,g,n,j]"
667  "Number of licenses argument missing"
668  "Number of licenses cannot be negative or zero"
669  "N"
670  "P"
671  "p"
$
$ NOTRANS.
672  "%%1$ld"
$
$ The next 24 messages are ADMIN.
$
673  "Product ID cannot be less than zero"
674  "r"
675  "s"
676  "S"
677  "t"
678  "o"
679  "h"
680  "v"
681  "w"
682  "z"
683  "No targets given"
684  "Negative value for number of targets is illegal"
685  "Number of licenses specified must be >= 1"
686  "-D, -E, and -S are only valid with compound password type"
687  "Vendor not found - please specify vendor id with -i option"
688  "Vendor not found - please specify vendor key with -k option"
689  "The specified vendor id (%%1$s)\ndoes not match the vendor id (%%2$s) found in\nthe product data base for the vendor %%3$s."
690  "The specified vendor key does not match the vendor key found in\nthe product data base for the vendor %%1$s."
691  "No vendors in database - please specify vendor id, name, and key with -i, -v, and -k option"
692  "product %%1$ld"
693  "Specify product id with -p option"
694  "Invalid password type"
695  "multi-usage parameter (-m) only works with Concurrent license type"
696  "Target ids are not applicable when target type is any"
$
$ The next 6 messages are NOTRANS.
$
697  "C"
698  "8"
699  "4"
$
$ Format for an internet address
700  "%%1$2X%%2$1X%%3$2X%%4$1X%%5$2X%%6$1X%%7$2X"
$
$ inet address, message
701  "%%1$s: %%2$s"
702  "%%1$x"
$
$ The next 3 messages are ADMIN.
$
703  "Number of licenses must equal number of targets for nodelocked and usage-metered passwords"
704  "Number of licenses must be >= to number of servers specified"
705  "    **** Developers Licenses ****\n"
$
$ The next 2 messages are NOTRANS.
706  "\n\n\n"
707  "b"
$
$ The next 63 messages are ADMIN.
$
708  "Start and expiration dates can not predate current date"
709  "Start date can not be more than 4095 days from current date"
710  "You have to call Gradient to get demo licenses"
711  "Expected a hex number"
712  "Date must be of the form YY/MM/DD or YY MM DD\n or YYYY/MM/DD or YYYY MM DD "
713  "duration is an integer between 1 and 36525 days"
714  "out of memory"
715  "Unable to obtain iFOR/LS licenses"
716  "Unable to obtain compound licenses from server"
717  "WARNING target id -1 same as ANY"
718  "WARNING this target id is derived from an internet address"
719  "-d and -e are mutually exclusive"
720  "Expected model:serial"
721  "Expected decimal number"
722  "Invalid key - expected decimal number"
723  "Invalid target type"
724  "Invalid license type"
725  "Invalid product id - expected decimal number"
726  "Not a valid uuid - use /etc/ncs/uuid_gen"
727  "Version must be 8 characters or less in length"
728  "aggregate duration can not exceed no. of licenses times duration"
729  "License Annotation cannot be entered when using supplier's compound password"
730  "License Annotation must be 80 characters or less in length"
731  "Vendor key must be a positive integer or the word 'supplier'"
732  "Could not create file: "
733  "Could not open file: "
734  "Vendor still has products"
735  "Total licenses not applicable to node locked and usage metering licenses"
736  "Name must be less than 80 characters"
737  "The nodelocked password for iFOR/LS licenses is invalid or not present"
738  "Not a valid uuid"
739  "Too many licenses per server - maximum is 65535"
740  "A product with this name, but a different id, exists"
741  "A product with this id, but a different name, exists"
742  "A product with this id and version already exists"
743  "A vendor with this id and name already exists"
744  "Too few licenses allocated here for given total"
745  "As many licenses as targets must be allocated"
746  "Too many licenses allocated here for given total"
747  "A vendor with this name exists"
748  "A vendor with this id, but a different name, exists"
749  "-a and -o are mutually exclusive"
750  "Invalid iFOR/LS license password"
751  "Could not create customer database"
752  "Missing arguments"
753  "Invalid number of licenses - expected decimal number"
754  "No product has been selected"
755  "Extra unexpected arguments detected"
756  "A vendor with this name, but a different id, exists"
757  "supplier"
758  "not available"
759  "Current Vendor: "
760  "Current Product: "
761  "maximum"
762  "any"
763  "supplier"
764  "Date must be of the form MM/DD/YY"
765  "******************"
766  "*servers*"
767  "Please Select a Vendor"
768  "Please Select a Product"
769  "Current Customer: "
770  "No Customer Information entered"
$
$ The next 2 messages are NOTRANS.
$
$ 	Format string for "Could not create file: ", filename
771  "%%1$s%%2$s"
$
$	Format string for "Current Product: ", product name, product version
772  "%%1$s%%2$s [%%3$s]"
$
$ The next 5 messages are ADMIN.
$
773  "   Count: %%1$d"
$
$ %1 is the current target and %2 is the total number of targets
774  "Target %%1$d of %%2$d"
775  "Target 1 of 1    "
776  "\nDialog interface usage:  ls_pass\n"
777  "?(ls_pass)  error: %%1$i invalid argument(s) preceeding graphic argument delimiter \"%%2$s\" :\n"
$
$ The next 4 messages are NOTRANS.
$
778  ">>    %%1$s\n"
779  "%%1$s\n"
780  "\0"
781  "?(ls_pass) %%1$s\n"
$
$ ADMIN message.
782  "   **** Developers Licenses ****\n"
$
$ The next 11 messages are NOTRANS.
$
783  "\n\n\n"
784  "%%1$x"
785  "c"
786  "C"
787  "8"
788  "4"
$
$ Format for an internet address
789  "%%1$2X%%2$1X%%3$2X%%4$1X%%5$2X%%6$1X%%7$2X"
$
$	Format string for "A product with this id, but a different name, exists",
$ product name
790  "%%1$s (%%2$s)"
$
$	Format string for "A product with this name, but different id, exists",
$ product id
791  "%%1$s (%%2$d)"
792  "r"
793  "a"
$
$ The next 3 messages are ADMIN.
$
794  "\n--------------------- N L S P A S S ---------------------\n\n"
795  "Customer name:  %%1$s\n"
796  "      Address:  %%1$s\n"
$
$ NOTRANS.
797  "                %%1$s\n"
$
$ The next 2 messages are ADMIN.
798  "      Contact:  %%1$s\n"
799  "Apollo"
$
$ The next 2 messages are NOTRANS.
800  "%%1$s [%%2$s]"
801  "\n"
$
$ The next 25 messages are ADMIN.
$
802  "Domain"
803  "Any"
804  "Sun"
805  "VAX/VMS"
806  "MS-DOS"
807  "HP-UX"
808  "Ultrix"
809  "HP-OSF"
810  "SVR4"
811  "Intergraph"
812  "SCO_UNIX"
813  "SGI"
814  "NeXT"
815  "AIX"
816  "Novell"
817  "DG-AViiON"
818  "Concurrent Access"
819  "Node Locked"
820  "Use Once"
821  "Compound"
822  "Usage Metering"
823  "?(ls_pass) %%1$s\n"
824  "Vendor: %%1$s,  Vendor id: %%2$s,  Vendor key: %%3$s\n"
825  "Vendor: %%1$s,  Vendor id: %%2$s,  Vendor key: %%3$d\n"
826  "Product: %%1$s, Product id: %%2$ld\n"
$
$ Name and version. NOTRANS.
827  "%%1$s [%%2$s]"
$
$ The next 14 messages are ADMIN.
$
828  "\nVendor Password %%1$s \n"
829  "built as\n"
830  "   vendor name : %%1$s\n"
831  "   vendor id   : %%1$s\n"
832  "   target      : any \n"
833  "   target      : %%1$x \n"
834  "\nProduct Passwords built as\n"
835  "   name         : %%1$s\n"
836  "   id           : %%1$ld\n"
837  "   version      : %%1$s\n"
838  "   password type: %%1$s\n"
839  "   license type : %%1$s\n"
840  "\n    Product password for "
841  "any"
$
$ The next 4 messages are NOTRANS.
$
$	Model and serial
842  "%%1$s:%%2$s"
843  "%%1$x"
844  "0x080009%%1$06x"
845  "0x%%1$x"
$
$ The next 12 messages are ADMIN.
$
846  " server : %%1$s\n"
847  "    built as\n"
848  "       license annotation : %%1$s\n"
849  "       start date         : "
850  "       duration           : %%1$d\n"
851  "       expiration         : "
852  "       number             : %%1$d\n"
853  "       multi usage rules  : Same"
854  " user"
855  " group"
856  " node"
857  " job"
$
$ NOTRANS.
858  ".\n"
$
$ The next 11 messages are ADMIN.
$
859  "       aggregate duration : %%1$d\n"
860  "       derived start      : "
861  "       derived expiration : "
862  "       license type       : %%1$s\n"
863  "       target type        : %%1$s\n"
864  "       target             : Any\n"
$
$	Model, serial and server id
865  "       target             : %%1$s:%%2$s  (0x%%3$x)\n"
866  "       target             : %%1$x\n"
867  "       target             : 0x080009%%1$06x\n"
$
$	Vendor name, vendor id, password
868  "ls_admin -a -v \"%%1$s\" %%2$s %%3$s\n\n\n"
869  "\nPut the following line into your nodelock file: \n\n"
$
$ The next 4 messages are NOTRANS.
$
$ Vendor id, password, and version
870  "%%1$s %%2$s \"\" \"%%3$s\"\n\n\n"
$
$ Vendor id, password, liocense annotation, and version
871  "%%1$s %%2$s \"%%3$s\" \"%%4$s\"\n\n\n"
$
$ Vendor name, product name, password, and version
872  "ls_admin -a -p \"%%1$s\" \"%%2$s\" %%3$s \"%%4$s\"\n"
$
$ License annotation, vendor name, product name, password, and version
873  "ls_admin -a -l \"%%1$s\" -p \"%%2$s\" \"%%3$s\" %%4$s \"%%5$s\"\n"
$
$ The next 77 messages are ADMIN.
$
874  "?(ls_pass) Warning: One or more servers were unable to be contacted.\n"
875  "                    iFOR/LS License count may be wrong.\n"
876  "\nYou now have %%1$d iFOR/LS use-once licenses available.\n\n"
877  "LS_PASS"
878  "XLs_Pass"
879  "Exit"
880  "Help Index..."
881  "Done"
882  "field"
883  "info"
884  "Add Vendor..."
885  "Add Product..."
886  "Show Vendor"
887  "Show Product"
888  "Delete..."
889  "iFOR/LS License Type:"
890  "Use-Once"
891  "Node Locked"
892  "Developer"
893  "Operations:"
894  "Operate On:"
895  "Vendor"
896  "Product"
897  "Password"
898  "Customer"
899  "ADD VENDOR"
900  "Vendor Name"
901  "Vendor Id"
902  "Vendor Key"
903  "Create Vendor Id"
904  "Vendor Password"
905  "ADD PRODUCT"
906  "Product Name"
907  "Product Id"
908  "Product Version"
909  "License Annotation"
910  "License Password"
911  "Customer Name"
912  "Address"
913  "Contact"
914  "Target Id"
915  "Target Type: "
916  "Next Target"
917  "Start"
918  "Start Date"
919  "Derived Start"
920  "Derived Start Date"
921  "Duration"
922  "Duration (days)"
923  "Derived Duration"
924  "Derived Duration (days)"
925  "Expiration"
926  "Expiration Date"
927  "Derived Expiration"
928  "Derived Expiration Date"
929  "Aggregate Duration"
930  "Aggregate Duration (days)"
931  "Number of licenses (this target)"
932  "Number of licenses"
933  "Number of licenses (total)"
934  "Create Script"
935  "Output file"
936  "Output file Name"
937  "Create passwords"
938  "Number of Targets"
939  "Multi-use rules:"
940  "Same User"
941  "Same Group"
942  "Same Node"
943  "Same Job"
944  "No Multi-use selection -- using supplier's compound password"
945  "No Annotation -- using supplier's compound password"
946  "CONFIRM"
947  "Delete"
948  "Delete Vendor?"
949  "Delete Product?"
950  "?(ls_pass)  warning: %%1$i invalid argument(s) following graphic argument delimiter \"%%2$s\" :\n"
$
$ NOTRANS.
951  ">>    %%1$s\n"
$
$ ADMIN.
952  "or  ls_pass %%1$s [ standard X arguments ]\n"
$
$ The next 2 messages are NOTRANS.
953  "s_v_add"
954  "s_p_add"
$
$ The next 25 messages are ADMIN.
$
955  "Domain"
956  "Hp-ux"
957  "SunOS"
958  "Intergraph"
959  "NeXT"
960  "SGI"
961  "SVR4"
962  "SCO_UNIX"
963  "MS-DOS"
964  "Ultrix"
965  "VAX/VMS"
966  "AIX"
967  "Novell"
968  "DG/UX"
969  "Any"
970  "License"
971  "Compound"
972  "Concurrent"
973  "Use Once"
974  "Usage Meter"
975  "Password Type:"
976  "License Type:"
977  "Vendors"
978  "Products"
979  "Customers"
$
$ NOTRANS.
980  "                 "
$
$ ------------------------------- Nodelock Routines ---------------------------
$
$ ADMIN message.
981  "entering: %%1$s\n"
$
$ The next 6 messages are NOTRANS.
$
982  "%%1$s\n"
983  "r"
984  "#"
985  "\0"
986  "\n"
987  "\t"
$
$ ADMIN.
988  "malloc() failed - out of memory\n"
$
$ NOTRANS.
989  "netls_get_nodelock_info()"
$
$ ------------------------------ Option Usage Messages ------------------------
$
$ The next 2 messages are ADMIN.
990  "usage:%%1$s"
991  "program"
$
$ The next 4 messages are NOTRANS.
$
992  "\0"
993  " %%1$s"
994  "\n"
995  "      %%1$s"
$
$ ADMIN. 
996  ":No usage data available"
$
$ The next 3 are NOTRANS.
997  ">"
998  "<"
999  "+"
$
$ The next 2 are ADMIN.
1000  "input_file"
1001  "output_file"
$
$ The next 2 are NOTRANS.
1002  "#"
1003  ":"
$
$ ADMIN.
1004  "string"
$
$ The next 48 messages are NOTRANS.
$
1005  "="
1006  " -"
1007  " -%%1$c"
$
$ -------------------------------- iFOR/LS working with GPR ---------------------
$
1008  "3957cd21c000.0d.00.00.da.b4.00.00.00"
1009  "1.0"
1010  "2.0"
1011  "%%1$s"
1012  "y"
1013  "yes"
1014  "Y"
1015  "YES"
1016  "n"
1017  "no"
1018  "N"
1019  "NO"
1020  "Please answer with yes or no (\"y\", \"yes\", \"n\", or \"no\")\n"
1021  "Obsolete versions of the locator product are present.\n"
1022  "Obsolete versions of the keyboard product are present.\n"
1023  "Your upgrade agreement requires that these licenses be deleted.\n"
1024  "Exiting.\n"
1025  "No locator licenses are currently available.\n  Do you want to wait (y/n)? "
1026  "No keyboard licenses are currently available.\n  Do you want to wait (y/n)? "
1027  "You are number %%1$d in the queue\n"
1028  "Waiting...\n"
1029  "Warning: your licenses will expire in less than 30 days\n"
1030  "You do not have any valid scribble licenses installed.  Exiting.\n"
1031  "You have licenses for the keyboard product only.\n"
1032  "You have licenses for the locator product only.\n"
1033  "Would you like to activate the keyboard (y/n)? "
1034  "Would you like to activate the locator (y/n)? "
1035  "Exiting...\n"
1036  "Application can not run without licenses.\n"
1037  "h"
1038  "j"
1039  "k"
1040  "l"
1041  "2.0.1 (GRI 1.1.2e) AIX"
$
$ -------------------------- Communication Utility Messages -------------------
$
1042  " "
1043  "-"
1044  "+"
1045  "%%1$d"
1046  "\0"
1047  "\n"
$
$ ------------------------------ UAI XMT Messages -----------------------------
$
1048  "%%1$s\n"
$
$ Format string for a message string followed by a variable
1049  "%%1$s: %%2$s\n"
1050  "I"
1051  "S"
1052  "#"
$
$ The next 8 messages are ADMIN.
$
1053  "Help"
1054  "HELP"
1055  "Help On"
1056  "Help Index"
1057  "Help Topics"
1058  "No help available"
1059  "Enter new value"
1060  "Edit the text or click 'Cancel' to\n\\return to the previous value.\n\\\n\\When finished, press:\n\\<return> to enter *only* the characters\n\\to the *left* of the insert cursor,\n\\or press <enter> or click on the 'OK'\n\\button to enter the new value completely."
$
$ The next 2 messages are NOTRANS.
1061  "\0"
1062  " "
$
$ The next 8 messages are ADMIN.
$
1063  "Enter new %%1$s"
1064  "double initialization of help index"
1065  "double initialization"
1066  "notify"
1067  "Dismiss"
1068  "unmapped_form"
1069  "NOTIFICATION"
1070  "Prompt Box"
$
$ NOTRANS.
1071  "r"
$
$ The next 27 messages are ADMIN.
$
1072  "bad help file -- won't open"
1073  "bad help file -- bad format"
1074  "nothing following .I line in"
1075  "no topics, only index in help file"
1076  "expected .S topics in help file, found"
1077  "form"
1078  "frame"
1079  "sep"
1080  "label"
1081  "push_btn"
1082  "radio_btn"
1083  "check_btn"
1084  "cascade_btn"
1085  "option_btn"
1086  "pulldown_menu"
1087  "radio_box"
1088  "selection_dialog"
1089  "prompt_dialog"
1090  "question_dialog"
1091  "dialog"
1092  "work"
1093  "pop_btn"
1094  "text_button"
1095  "Why not?"
1096  "list"
1097  "scrolled_text"
1098  "topic not found in help file"
$
$ ------------------------- License Server Utilty Messages --------------------
$
$ The next 5 messages are NOTRANS.
$
1099  "length of buffer not a multiple of 8 in encrypt_key() (size = %%1$d)\n"
1100  "length of buffer not a multiple of 8 in decrypt_key() (size = %%1$d)\n"
1101  "abcd"
1102  "/"
1103  " "
$
$       Year, month, day. ADMIN message.
1104  "%%1$d %%2$d %%3$d"
$
$       Year, month, day. ADMIN message.
1105  "%%1$d/%%2$d/%%3$d"
$
$ The next 14 messages are NOTRANS.
$
$       Text, Date
1106  "%%1$s%%2$s\n"
1107  "%%1$s\n"
$
$
$ ------------------------------- License Server Version Messages -------------
$
1108  "2.0.1 GR1.0.7a Apollo Domain"
1109  "2.0.1 (GR1.0.7a) Intergraph"
1110  "2.0.1 (GR1.0.7a) SVR4"
1111  "2.0.1 (GR1.0.7a) SCO_UNIX"
1112  "2.0.1 (GR1.0.7a) HP-UX"
1113  "2.0.1 (GR1.0.7a) DEC-ULTRIX"
1114  "2.0.1 (GR1.0.7a) DEC-VMS"
1115  "2.0.1 (GR1.0.7a) MIPS-RISCOS"
1116  "2.0.1 (GR1.0.7a) SunOS"
1117  "2.0.1 (GR1.0.7a) SGI"
1118  "2.0.1 (GR1.0.7a) NeXT"
1119  "2.0.1 (GR1.0.7a) OSINFO"
$
$
$ ------------------- 4 more messages for nlspass_cmd.c -----------------------
$
$ The next 5 messages are ADMIN.
$
1120  "    [-e expiration] (mm/dd/yy)\n"
1121  "    [-E derived expiration] (mm/dd/yy)\n"
1122  "    [-s start] (mm/dd/yy)\n"
1123  "    [-S derived start] (mm/dd/yy)\n"
1124  "Cannot specify a target type or id of ANY whne getting license from a compound license."
$
$ More messages for nl_test.  These are DEVELOP messages.
$
1125  "Novell"
1126  "DG/UX"
1127  "OSF/1"
1128  "OS/2"
1129  "OS/400"
1130  "MVS"
1131  "Win/NT"
$
$ More messages for the pass tool. These are DEVELOP messages.
$
1132  "                osf1, os2, sco,\n"
1133  "f"
1134  "2"
1135  "HP-OSF"
1136  "MVS"
1137  "OSF/1"
1138  "OS/2"
1139  "OS/400"
1140  "Win/NT"
1141  "OSF/1"
1142  "OS/2"
1143  "OS/400"
1144  "MVS"
1145  "Win/NT"
