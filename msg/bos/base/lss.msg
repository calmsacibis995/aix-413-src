$ ENGLISH_SOURCE_VERSION= 1.8
$ @(#)53        1.6  src/msg/bos/base/lss.msg, bosmsg, msg411, 9438B411a 9/23/94 10:06:40
$
$   COMPONENT_NAME: libncs
$
$   FUNCTIONS:
$
$   ORIGINS: 84
$
$ -----------------------------------------------------------------------------------------
$
$ lss.msg
$
$ Message source object file used by the gencat utility for message catalogue generation.
$ It is a compilation of catalogued messages that were extracted from from the source code
$ modules that comprise the following utilities:
$
$					ls_admin
$					ls_rpt
$					ls_stat
$					ls_target_id
$					ls_test
$					ls_tv
$
$ -----------------------------------------------------------------------------------------
$
$quote "
$delset 2
$set 2
$
$
$ 	----------------------------- Cleanup Server Messages -----------------------------
$
$ The first 7 messages are DEVELOP.
$
1  "   object = *\n"
2  "   object = %%1$s\n"
3  "     type = *\n"
4  "     type = %%1$s\n"
5  "interface = *\n"
6  "interface = %%1$s\n"
7  "Unregistering the following location broker entry:\n"
$
$ The next 2 messages are NOTRANS.
8  "------------\n"
9  "%%1$s\n"
$
$ The next 10 messages are DEVELOP.
10  "Testing matching location broker entries:\n"
11  "Unregistering matching location broker entries:\n"
12  "number of good local servers = %%1$d\n"
$
$ ---------------------------- Data-Store Memory Messages ---------------------
$
13  "dsm__print_map unavailable.\n"
14  "DSM map; %%1$d initialized pages of %%2$d bytes; %%3$d allocations pending\n"
15  "---- %%1$d byte chunk at %%2$d\n"
$
$	Datastore memory message output. First block of memory, location, page,
$ offset and bytes.
16  "    %%1$08x %%2$08x: (page %%3$d offset %%4$d) %%5$d bytes "
17  " *** NOT 8-BYTE ALIGNED *** "
18  "(free)"
19  " *** INVALID COOKIE *** "
$
$ The next 51 messages are NOTRANS.
$
20  "\n"
$
$ ------------------------------- Get HP ID  Messages -------------------------
$
21  "@(#) $Revision: 1.1 $ \n"
22  "\0"
23  "%%1$02x"
24  "/"
25  "???"
26  "840"
27  "0"
28  "\020"
29  "1"
$
$       HP node-id model number and model number letter for model number 1.
30  "%%1$5.5u-6990%%2$c"
$
$       HP node-id model number and model number letter.
31  "%%1$5.5u%%2$c"
$
$  HP node-id serial date, country and serial index.
32  "%%1$4.4u%%2$c%%3$5.5u"
$
$       Format for scanf for the first part and second part of the HP node-id.
33  "%%1$5s %%2$x"
34  "SPU"
35  "000000000000"
36  "LLA"
37  "9000/7"
38  " "
39  "standalone"
40  "+"
41  "_isc_table"
42  "stderr, nlist: failed\n"
43  "open"
44  "lseek"
45  "read"
46  "%%1$x"
$
$ ------------------------------- Get SVR4 ID Messages ------------------------
$
47  "%%1$d"
48  "\0"
49  "/"
$
$ %1 is "/hinv/enet", %2 is the device name, %3 is the extension
50  "%%1$s/%%2$s/%%3$s"
51  ".attr"
52  "r"
53  "\n"
54  "net_addr"
55  "0"
56  "%%1$x"
57  ":"
58  "dlpi_req putmsg"
59  "dlpi_req getmsg"
$
$	Error message format string for failed reply from a sent request to the
$ dlpi device when querying the SVR4 node id.
60  "dlpi_req prim=%%1$x/%%2$x errno=%%3$d unix_errno=%%4$d\n"
61  "get_my_ether_addr: socket"
62  "get_my_ether_addr: ioctl (SIOCGIFCONF)"
63  "ioctl (SIOCGIFFLAGS)"
64  "ioctl (SIOCRPHYSADDR)"
$
$ ----------------------------- Get Utility Option Messages -------------------
$
65  "="
66  "\0"
67  "/"
68  ";"
69  "]"
70  ":"
$
$ The next 3 messages are ADMIN.
71  "nextopt() error: no known options\n"
72  "nextopt() error: no arguments to parse\n"
73  "use \""
$
$ NOTRANS
74  "%%1$s "
$
$ ADMIN.
75  "-usage\" to get usage\n"
$
$ The next 5 messages are NOTRANS.
$
76  "0"
77  "9"
78  "+"
79  "-"
80  "?"
$
$ The next 2 messages are ADMIN.
81  "%%1$s: illegal option: %%2$c\n"
82  "%%1$s: illegal option -- %%2$c\n"
$
$ NOTRANS.
83  "["
$
$ The next 5 messages are ADMIN.
$
84  "usage"
85  "version"
86  "No known version"
87  "%%1$s: unknown option: %%2$s\n"
88  "option %%1$s ambiguous: matches"
$
$ The next 3 messages are NOTRANS.
89  "\n"
90  " ="
91  "<geometry-spec>"
$
$ ADMIN.
92  "option requires an argument --"
$
$ NOTRANS.
93  "#"
$
$ The next 2 messages are ADMIN.
94  "option"
95  " expected integer argument\n"
$
$ The next 6 messages are NOTRANS.
$
96  ">"
97  "w"
98  "a"
99  "<"
100  "r"
101  ": "
$
$ ADMIN.
102  "Unknown code in options: %%1$c\n"
$
$ NOTRANS.
103  " %%1$c"
$
$ -------------------------------- Log File Messages --------------------------
$
$ The next 3 messages are ADMIN.
104  "Could not open or create log file: %%1$s"
105  "Could not open log file: %%1$s"
106  "Current log file has wrong version\n"
$
$ The next 6 messages are NOTRANS.
$
107  "cleanup_log"
108  ".new"
109  "."
110  "new;"
111  ";"
112  "****"
$
$ --------------------------- Machine Utitility Messages ----------------------
$
$ ADMIN message.
113  "problems creating cache: "
$
$ The next 3 messages are NOTRANS.
114  "/"
115  ":"
116  "\0"
$
$ ADMIN.
117  "Unrecognized protocol family: %%1$s"
$
$ NOTRANS.
118  "."
$
$ The next 4 messages are ADMIN.
$
119  "Can't communicate with protocol family: %%1$s"
120  "problems getting lb entry data"
121  "Duplicate fully qualified nodenames match %%1$s\n"
122  "unknown"
$
$ --------------------------- License Server Messages -------------------------
$
$ DEVELOP.
123  "**** entering: %%1$s() ****\n"
$
$ The next 2 messages are NOTRANS.
124  "   %%1$f\n"
125  "ls_Request_License"
$
$ DEVELOP.
126  "ls_Request"
$
$ NOTRANS.
127  "   "
$
$ The next 4 messages are DEVELOP.
$
128  "ls_Request_License_11"
129  "ls_Request_And_Wait_11"
130  "ls_Req_And_Wait"
131  "ls_Request_Compound"
$
$ The next 2 messages are ADMIN.
132  "using compound license"
133  "unknown"
$ 
$ NOTRANS.
134  "\0"
$
$ The next 4 messages are DEVELOP.
$
135  "ls_Wait_Status"
136  "ls_Wait_Status_11"
137  "ls_Wait_Stat"
138  "ls_Wait_Remove"
$
$ NOTRANS
139  "  "
$
$ The next 4 messages are DEVELOP.
$
140  "ls_Wait_Remove10"
141  "ls_Release"
142  "ls_Release10"
143  "ls_Check"
$ 
$ NOTRANS.
144  " "
$
$ The next 11 messages are DEVELOP.
$
145  "ls_Check10"
146  "ls_Get_Cur_Users"
147  "ls_Get_Cur_Users10"
148  "ls_Log_Msg"
149  "ls_Log_Msg10"
150  "ls_get_wait_q_sz"
151  "ls_get_wait_q_sz10"
152  "ls_parse_user_file"
153  "ls_parse_user_file10"
154  "ls_stale_lic_chk"
155  "ls_Get_Svr_Info"
$
$ The next 12 messages are NOTRANS.
$
156  ":"
157  "//"
$
$	Domain operating system revision information.  Output numbers 
$	correspond to major and minor revision numbers.
158  "DomainOS SR %%1$d.%%2$d"
159  "HP-UX"
160  "SunOS"
161  "SVR4"
162  "Intergraph"
163  "NeXT"
164  "SCO_UNIX"
165  "MS-DOS"
166  "Ultrix"
167  "VMS"
$
$ ADMIN.
168  "AIX"
$
$ DEVELOP.
169  "ls_License_Already_Granted"
$
$ NOTRANS.
170  "SGI_IRIX"
$
$ ------------------------- License Server Command Messages -------------------
$
$ The next 3 messages are DEVELOP.
171  "**** entering: %%1$s() ****\n"
172  "ls_get_server_locking"
173  "ls_add_vendor"
$
$ NOTRANS.
174  ": "
$ 
$ ADMIN.
175  "too many vendors on this db - delete some please"
$
$ The next 6 messages are DEVELOP.
$
176  "ls_add_vendor_safely"
177  "ls_get_vendors"
178  "ls_get_vendors_safely"
179  "ls_delete_vendor"
180  "ls_rename_vendor"
181  "ls_add_product"
$
$ NOTRANS.
182  "\0"
$ 
$ The next 15 messages are DEVELOP.
$
183  "ls_add_product_int"
184  "ls_add_product_safely"
185  "ls_add_product_10"
186  "ls_delete_product"
187  "ls_rename_product"
188  "ls_get_all_products2"
189  "ls_get_all_products"
190  "ls_get_all_products_10"
191  "ls_get_products2"
192  "ls_get_products"
193  "ls_get_products_10"
194  "ls_get_vendor_by_name"
195  "ls_get_product_by_name"
196  "ls_get_product_by_name_10"
197  "ls_nls_get_info"
$
$ NOTRANS.
198  "ls_get_nls_info"
$
$ The next 2 messages are DEVELOP.
199  "ls_copy_vendor"
$
$ ---------------------------- License Database Messages ----------------------
$
200  "entering: %%1$s()\n"
$
$ NOTRANS.
201  "   %%1$f\n"
$
$ The next messages are ADMIN.
$
202  "Database read error -- errno: %%1$d\n"
203  "Database write error -- errno: %%1$d\n"
204  "Bad source or could not open or create database: %%1$s\n"
$
$ The next 4 messages are DEVELOP.
$
205  "init_db"
206  "sizeof(prod_info_t) = %%1$d\n"
207  "sizeof(lic_entry_t) = %%1$d\n"
208  "sizeof(v_hdr_t) = %%1$d\n"
$
$ ADMIN.
209  "Malloc error in init_db\n"
$
$ The next 8 messages are NOTRANS.
$
210  "."
211  "new"
212  "bak"
213  "tmp"
214  "new;"
215  "bak;"
216  "tmp;"
$
$ Version and hostid
217  " %%1$s; %%2$x"
$
$ The next 10 messages are ADMIN.
$
218  "Could not open or create current info database: %%1$s\n"
219  "Could not create database: %%1$s\n"
220  "? (nlsd) 1.1 license database being converted to 2.0 format ...\n"
221  "Apollo"
222  "1.1 license database being converted to 2.0 format"
223  "? (nlsd) conversion complete.\n"
$
$	The following message printed as the resulted of an unexpected license
$	database version id. 
224  "Wrong version in license database %%1$s - expected %%2$d found %%3$d\n"
225  "License database header is corrupt\n"
226  "License database is corrupt\n"
227  "License database on an invalid node\n"
$
$ The next 2 messages are DEVELOP.
228  "close_db"
229  "init_caches"
$
$	The following message message is an internal error message generated 
$ when there is	not enough memory to generate the product information table.
$ ADMIN.
230  "Out of memory in init_caches - vnd = %%1$d; num_products = %%2$d\n"
$
$ NOTRANS.
231  "\0"
$
$ The next 39 messages are DEVELOP.
$
232  "update_vnd_cache"
233  "get_vendor_int"
234  "get_vendor"
235  "put_vendor"
236  "add_vendor"
237  "delete_vendor"
238  "get_products"
239  "get_productsr"
240  "put_product"
241  "add_product"
242  "delete_product"
243  "get_compound_license_info"
244  "get_useonce_license"
245  "rename_product"
246  "get_license_entry"
247  "read_license_entry_int"
248  "read_license_entry"
249  "write_license_entry"
250  "delete_license_entry"
251  "get_lic_entries"
252  "get_vendor_by_name"
253  "get_product_by_name"
254  "get_all_vendors"
255  "get_all_products"
256  "get_cur_info"
257  "sync_cur_vs_prod"
258  "cur_in_use < prod_in_use\n"
259  "prod_in_use < cur_in_use\n"
260  "change_db"
$
$ ADMIN.
261  "Could not open temporary license database for update: %%1$s\n"
$
$ The next 5 messages are DEVELOP.
$
262  "change_db_commit"
263  "revert_db"
264  "write_all_products_int"
265  "db_fsync"
266  "convert_lic_database"
$
$	Output format string for license database name and database version.
$ NOTRANS.
267  "%%1$s.%%2$d"
$
$ The next 5 messages are ADMIN.
$
268  "Converting file %%1$s to %%2$s\n"
269  "Could not open 1.1 license database during conversion: %%1$s\n"
270  "Could not open license database during conversion: %%1$s\n"
271  "Could not create license database during conversion: %%1$s\n"
272  "License Database Conversion type %%1$d not found\n"
$
$	Output string formatted for vendor: vendor name (num of products. 
$ DEVELOP.
273  "    converting vendor %%1$d: %%2$s (num of prods = %%3$d)\n"
$
$ ADMIN.
274  "Could not add vendor %%1$s during license database conversion\n"
$
$	Output format string for current product: old product. DEVELOP.
275  "        converting product %%1$d: %%2$s\n"
$
$	Output format string for product string and vendor. ADMIN.
276  "Could not add product %%1$s to vendor %%2$s during license database conversion\n"
$
$ The next 2 messages are DEVELOP.
277  "checksum_structure"
278  "checksum_whole_file"
$
$ -------------------------------- License Server -----------------------------
$
$ ADMIN.
279  "netlsd"
$
$ The next 6 messages are DEVELOP.
$
280  "hpterm -fg wheat -bg DarkSlateGrey -n %%1$s -e xdb %%2$s -P %%3$d"
281  "hello"
282  "/bin/sh"
283  "sh"
284  "-c"
285  "could not exec new window\n"
$
$ NOTRANS.
286  "/"
$
$ The next 2 messages are ADMIN.
287  "Terminating...\n"
288  "warning: um security task not stopped\n"
$
$ The next 6 messages are NOTRANS.
$
289  "\0"
290  "`node_data"
291  "."
292  "A"
293  "Z"
294  "a"
$
$ ADMIN.
295  "netlsd: This program requires super user privileges\n"
$
$ NOTRANS.
296  "?"
$
$ The next 2 messages are ADMIN.
297  "-usage"
298  "-u"
$
$ NOTRANS.
299  "NETLSD$ARGS"
$
$ The next 2 messages are DEVELOP.
300  "-v"
301  "-verbose"
$
$ The next 4 messages are ADMIN.
$
302  "-p"
303  "-perf"
304  "-o"
305  "-override"
$
$ DEVELOP.
306  "-z"
$
$ The next 3 messages are ADMIN.
307  "-n"
308  "-no"
309  "netlsd: netlsd already running.\n"
$
$ The next 2 messages are DEVELOP.
310  "netlsd version %%1$s\n"
311  "netlsd host id is %%1$x\n"
$
$ The next 9 messages are ADMIN.
$
312  "Could not find any supported address families"
313  "Unable to register ls10 rpc interface"
314  "Unable to register ls rpc interface"
315  "Unable to register ls_cmds rpc interface"
316  "Unable to register ls_cmds version 1.0 rpc interface"
317  "Unable to register ls_log rpc interface"
318  "Unable to register ls_log20 rpc interface"
319  "Unable to register um rpc interface"
320  "Unable to register with locating broker"
$
$ NOTRANS.
321  ": "
$
$ ADMIN.
322  "Too many vendors in this db - delete some please"
$
$ DEVELOP.
323  "Listening...\n"
$
$ The next 4 messages are ADMIN.
$
324  "Rpc listen failed"
325  "netlsd [-o[verride]] [-n[o] %%1$s]\n"
326  "Unrecognized or redundant argument"
327  "The -no option must be followed by one or more of [%%1$s]"
$
$ The next 6 messages are NOTRANS.
$
$ The next 2 lines are for error messages
328  "?(netlsd) %%1$s\n"
329  "?(netlsd) %%1$s %%2$s\n"
330  "NETLSD"
331  "NETLSD pid=%%1$x\n"
332  "kill"
333  "lib$getjpi"
$
$ The next 12 messages are DEVELOP.
$
334  "The version of idl_base.h is not compatible with this stub/switch code."
335  "The version of rpc.idl is not compatible with this stub/switch code."
336  "The version of ncastat.idl is not compatible with this stub/switch code."
337  "The version of idl_base.h is not compatible with this stub/switch code."
338  "The version of rpc.idl is not compatible with this stub/switch code."
339  "The version of ncastat.idl is not compatible with this stub/switch code."
340  "The version of idl_base.h is not compatible with this stub/switch code."
341  "The version of rpc.idl is not compatible with this stub/switch code."
342  "The version of ncastat.idl is not compatible with this stub/switch code."
343  "The version of idl_base.h is not compatible with this stub/switch code."
344  "The version of rpc.idl is not compatible with this stub/switch code."
345  "The version of ncastat.idl is not compatible with this stub/switch code."
$
$ --------------------------- Internal Functional Messages --------------------
$
$ DEVELOP.
346  "entering: %%1$s()\n"
$
$ NOTRANS.
347  "   %%1$f\n"
$
$ DEVELOP.
348  "init_vnd_info"
$
$ ADMIN.
349  "malloc() ran out of memory"
$
$ The next 6 messages are DEVELOP.
$
350  "update_vnd_info"
351  "update_prod_info"
352  "get_vnd_info"
353  "get_prod_info"
354  "get_hash_table"
355  "hash"
$
$ NOTRANS.
356  "\0"
$
$ The next 5 messages are DEVELOP.
$
357  "user_auth_check"
358  "wait_add"
359  "wait_delete"
360  "get_wait_q_sz"
361  "write_cur_info"
$
$ NOTRANS.
362  "****"
$
$ The next 5 messages are DEVELOP.
$
363  "release_lics"
364  "request_lics"
365  "stale_lic_chk"
366  "exp_start_date_chk"
367  "check_for_multiple"
$
$ ------------------------------ ls_target_id Messages ------------------------
$
$ The next 22 messages are NOTRANS.
$
368  "@(#) $Revision: 1.1 $ \n"
369  "ls_targetid"
370  "%%1$x"
371  "              0x%%1$08x        SPU machine idetification number\n"
372  "%%1$x\n"
373  "000000000000"
374  "%%1$02x"
$
$	Format string for output: id, mode and serial. 
$
375  "              0x%%1$08x        ID MODULE: %%2$6s %%3$10s\n"
376  "            ?: "
377  "cannot create %%1$s -  must be super-user\n"
378  "cannot read %%1$s\n"
$
$	Format for lan card id and id numbers. 
379  "              0x%%1$08x        %%2$s LLA: 0x%%3$02x%%4$02x%%5$02x%%6$02x%%7$02x%%8$02x\n"
380  "%%1$s: must be super-user\n"
$
$	Format for lan card id and id numbers.
381  "\nCurrent iFOR/LS Target ID: 0x%%1$08x  (LLA: 0x%%2$02x%%3$02x%%4$02x%%5$02x%%6$02x%%7$02x)\n\n"
$
$ 	Format output for device file name and device file number. 
382  "%%1$s.%%2$d"
$
$ 	%1 is the name of the program and %2 is a device file name. 
383  "%%1$s: cannot create %%2$s\n"
$
$	Format for name and device file.
384  "%%1$s: cannot read lanic addr for %%2$s\n"
$
$	Format for a network address
385  "%%1$d. lan%%2$d (0x%%3$02x%%4$02x%%5$02x%%6$02x%%7$02x%%8$02x)\n"
386  "Select number for the new iFOR/LS Target ID: "
387  "%%1$d"
$
$	Lan formatted output string.
388  "\nNew iFOR/LS Target ID: 0x%%1$08x  (LLA: 0x%%2$02x%%3$02x%%4$02x%%5$02x%%6$02x%%7$02x)\n"
389  "%%1$s: no such device - target id not changed\n"
$
$ The next 5 messages are ADMIN.
$
390  "-s"
391  "-n"
$
$	Format output string for usage.
392  "usage:\n    %%1$s <-c>\n    %%2$s <-s>\n"
393  "    %%1$s <-n> <nodelocked index>\n"
394  "    %%1$s <-c>\n"
$
$ The next 14 messages are NOTRANS.
$
395  "\n"
396  "                 iFOR/LS Target ID for Usage Based Licenses\n"
397  "                 ----------------------------------------\n"
398  "For usage based iFOR/LS licenses (Concurrent or Use Once), report the the\nindicated iFOR/LS Target ID.  This ID is based on the link level address of the\nLAN card whose device file is %%1$s.\n\n"
399  "            iFOR/LS Target ID     Hardware ID source\n"
400  "            ---------------     ------------------\n"
401  "\n\n\n\n"
402  "                 iFOR/LS Target ID for NodeLock Licenses\n"
403  "                 -------------------------------------\n"
404  "For a iFOR/LS nodelock license, report one of the indicated iFOR/LS Target IDs.\n\n"
405  "cChH"
406  "c"
407  "C"
408  "?"
$
$ The next 2 messages are ADMIN.
409  "e:i:hH"
410  "e"
$
$ The next 2 messages are NOTRANS.
411  "\0"
412  ":"
$
$ NOTRANS.
413  "i"
$
$ The next 3 messages are ADMIN.
414  "usage:\n"
415  "    %%1$s -e <etheraddr> (xx:xx:xx:xx:xx:xx)\n"
416  "    %%1$s -i <hostid>\n"
$
$ NOTRANS.
417  "    %%1$s\n"
$
$ The next 2 messages are ADMIN.
418  "            PREFERRED iFOR/LS Target ID (CPU Planar ID)\n"
419  "            -----------------------------------------\n"
$
$	Formatted output string for hi, mid and low parts of the target id.
$ NOTRANS.
420  "            %%1$X%%2$X\n"
$
$ ADMIN.
421  "*** ls_targetid not implemented yet for your platform\n"
$
$ NOTRANS.
422  "	            %%X\n"
$
$ The next 4 messages are ADMIN.
$
423  "\n\n             ALTERNATE Target ID (Link Level Address)\n"
424  "            ---------------------------------------------\n"
425  "                 iFOR/LS Target ID\n"
426  "                ---------------\n"
$
$ -------------------------------- ls_test Messages ---------------------------
$
$ The next 26 messages are NOTRANS.
$
427  "ls_test: This program requires super user privileges\n"
428  "lic_db:\n"
429  "cur_db:\n"
430  "hdr length\t= %%1$d\n"
431  "num vendors\t= %%1$d\n"
432  "db version\t= %%1$d\n"
433  "cur_db next_hdr\t= %%1$d\n"
434  "cur_db used_count = %%1$d\n"
435  "cur_db first_num = %%1$d\n"
436  "node type\t= %%1$d\n"
437  "node id\t\t= 0x%%1$lx\n"
438  "lic_db uuid\t= %%1$s\n"
439  "lic_db hdr_chk\t= %%1$d\n"
440  "lic_db checksum\t= %%1$d\n"
441  "\n"
442  "\0"
443  "`node_data"
444  "."
445  "/"
446  "A"
447  "Z"
448  "a"
449  "?(netlsd) %%1$s\n"
$
$ Formatted error output for error message and corresponding status code.
450  "?(netlsd) %%1$s %%2$s\n"
$
$ ----------------------------------- I18N message routines -------------------
$
451  "%%1$s"
$
$ 	Filename and message
452  "%%1$s: %%2$s"
$
$ --------------------------------- ls_tv Messages ----------------------------
$
$ The next 5 messages are ADMIN.
$
453  "=usage"
454  " =usage option not valid for ls_tv\n\n"
455  "-usage"
456  "-h"
457  "-version"
$
$ The next 2 messages are NOTRANS.
458  "init"
459  "request_license"
$
$ NOTRANS.
460  " "
$
$ ADMIN.
461  "unknown"
$
$ The next 2 messages are NOTRANS.
462  ";"
463  "\0"
$
$ ADMIN.
464  "Failed license transaction on node %%1$s\n"
$
$ NOTRANS.
465  "release_license"
$
$ The next 4 messages are ADMIN.
$
$	Format output string for node id and license annotation.
466  "Completed license transaction on node %%1$s running iFOR/LS %%2$s"
467  "Active iFOR/LS Servers:"
468  "No servers found"
$
$	Format output string for server name, platform, and version numbers.
469  "    %%1$s (%%2$s) running iFOR/LS %%3$s\n"
$
$ The next 2 messages are NOTRANS.
470  "?(ls_tv) %%1$s\n"
471  "%%1$s\n"
$
$ The next 26 messages are ADMIN.
$
472  "usage: ls_tv [-h] [-usage] [-version]\n"
473  "LS_TV Version %%1$s -- iFOR/LS Test and Verification Tool\n"
474  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
475  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
$
$ ------------------------------- ls_admin Messages ---------------------------
$
476  "ls_admin"
477  "Domain"
478  "Any"
479  "Sun"
480  "VAX/VMS"
481  "MS-DOS"
482  "HP-UX"
483  "Ultrix"
484  "HP-OSF"
485  "SVR4"
486  "Intergraph"
487  "SCO_UNIX"
488  "SGI"
489  "NeXT"
490  "AIX"
491  "Novell"
492  "DG-AViiON"
493  "Concurrent Access"
494  "Node Locked"
495  "Use Once"
496  "Compound"
497  "Usage Metering"
$ 
$ NOTRANS.
498  "%%1$s\n"
$
$ ADMIN.
499  "Server: %%1$s"
$
$ The next 2 messages are NOTRANS.
500  ":"
501  "#"
$
$ The next 3 messages are ADMIN.
502  "    socket address family  %%1$s"
503  "    socket address         %%1$s"
504  "    socket port            %%1$d"
$
$ NOTRANS.
505  "%%1$x"
$
$ The next 4 messages are ADMIN.
$
506  "    target type            %%1$s"
507  "    target id              %%1$s"
508  "Vendor: %%1$s"
509  "    vendor id         %%1$s"
$
$ NOTRANS.
510  "\n"
$
$ The next 10 messages are NOTRANS.
$
$	Format output string for application name and version.
511  "Product: %%1$s [%%2$s]\n"
512  "    id              %%1$d\n"
513  "    License:"
514  "    Licenses:"
515  "          annotation:  %%1$s\n"
516  "          multi-use rules:  same"
517  " user"
518  " group"
519  " node"
520  " job"
$
$ NOTRANS.
521  ".\n"
$
$ The next 28 messages are ADMIN.
$
522  "expired: "
$
$ 	Formatted output strings for total licences, from date, to date, and 
$	timestamp.
523  "        %%1$d Concurrent Access from %%2$s to %%3$s, timestamp %%4$d\n"
524  "        %%1$d Use Once from %%2$s to %%3$s, timestamp %%4$d\n"
525  "        %%1$d Compound from %%2$s to %%3$s, timestamp %%4$d\n"
526  "          aggregate duration: %%1$d license type: %%2$s\n"
527  "          derived start: %%1$s derived expiration: %%2$s\n"
528  "        Usage Metering from %%1$s to %%2$s, timestamp %%3$d\n"
529  "-usage"
530  "-h"
531  "-version"
532  "LS_ADMIN Version %%1$s\n"
533  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
534  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
$
$ ------------------------------ ls_admin CLI Messages ------------------------
$
535  "Invalid vendor id (uuid)"
536  "Password required"
537  "Vendor id (uuid) required"
538  "Server not found"
539  "-p and -v are mutually exclusive"
540  "Vendor name required"
541  "Product name required"
542  "Delete product requires a timestamp"
543  "Timestamps must be decimal numbers"
544  "Version argument not allowed when product name not specified"
545  "Version must be 8 characters or less in length"
546  "License Annotation must be 80 characters or less in length"
547  "From server not found"
548  "Copy <product> From not supported"
549  "Copy <vendor> From is not valid with -s[who] or -d[elete]"
$
$ NOTRANS.
550  "?(ls_admin) %%1$s\n"
$
$ The next 6 messages are ADMIN.
$
551  "\nCommand line usage: ls_admin [-n <node name>] [-f <node name>] [-r <version number>]\n"
552  "       {-a | -s | -d}\n"
553  "       [{-v | -p} argument1 argument2 ....]\n"
554  "-v or -p must occur last in the command line.  The arguments to the option\n"
555  "should follow.  If anything else appears on the command line after these \n"
556  "options it will be ignored.\n"
$
$ NOTRANS.
557  "\0"
$
$ The next 11 messages are ADMIN.
$
558  "adf:l:n:pr:sv"
559  "a"
560  "d"
561  "f"
562  "l"
563  "n"
564  "p"
565  "r"
566  "missing version argument"
567  "s"
568  "v"
$
$ NOTRANS.
569  "%%1$d"
$
$ ---------------------------- ls_admin Dialog Messages -----------------------
$
$ The next 13 messages are ADMIN.
$
570  "Could not add this vendor"
571  "This server has no vendors"
572  "This vendor already exists"
573  "Invalid vendor id (uuid)"
574  "Supplied string is too long"
575  "A vendor already exists under this name"
576  "Could not add this product"
577  "This vendor still has products!"
578  "This vendor has no products"
579  "Product list temporarily unavailable from server"
580  "License list temporarily unavailable from server"
581  "Version must be 11 characters or less in length"
582  "License Annotation must be 80 characters or less in length"
$
$ The next 4 messages are NOTRANS.
$
583  "?(ls_admin) "
$
$	Format output for error message header and error message.
584  "%%1$s%%2$s\n"
585  " "
586  "\n"
$
$ The next message is ADMIN.
587  "\nDialog interface usage:  ls_admin\n"
$
$ NOTRANS.
588  "\0"
$
$	Format string for error header and error message. ADMIN.
589  "%%1$serror: %%2$i invalid argument(s) preceeding graphic argument delimiter \"%%3$s\" :\n"
$
$ NOTRANS.
590  ">>    %%1$s\n"
$
$ The next 16 messages are ADMIN.
$
591  "Unable to locate any Network License Servers"
592  "Consulting server"
593  "No servers found"
594  "Unable to communicate with new server"
595  "Reverted to previous server."
596  "Connected to server: %%1$s"
597  "Consulting with all iFOR/LS servers"
598  "Update complete"
599  "No Vendors"
600  "No Products"
601  "user_file is valid"
602  "Copy %%1$s to "
603  "Copied vendor %%1$s from %%2$s to %%3$s"
604  "Added vendor %%1$s, named \"%%2$s\""
605  "Renamed vendor %%1$s to %%2$s"
606  "Deleted vendor %%1$s"
$
$ 	Product name and version. NOTRANS.
607  "%%1$s [%%2$s]"
$
$ The next 14 messages are ADMIN.
$
608  "ca"
609  "uo"
610  "com"
$
$	Format output string for license type, from, to.
611  "%%1$d %%2$s from %%3$s to %%4$s"
$
$	Format output string for license type, from, to.
612  "um from %%1$s to %%2$s"
613  "Concurrent Access"
614  "Use Once"
615  "Compound"
616  "Usage Metering"
$
$	Format output string for vendor name, application name, version, number
$	of licences, license type, from date, to date and timestamp.
617  "Added product to vendor %%1$s: %%2$s [%%3$s], %%4$d %%5$s licenses from %%6$s to %%7$s, timestamp %%8$d"
$
$	Format output string for vendor name, application name, version, 
$	license type, from date, to date, and timestamp.
618  "Added product to vendor %%1$s: %%2$s [%%3$s], %%4$s license from %%5$s to %%6$s, timestamp %%7$d"
$
$	Format string for vendor name, old name, and application name.
619  "Renamed %%1$s product %%2$s to %%3$s"
$
$	Format string for vendor name, application name, version, number of 
$	licenses,  license type, from date, to date, time stamp.
620  "Deleted license from vendor %%1$s: %%2$s [%%3$s], %%4$d %%5$s licenses from %%6$s to %%7$s, timestamp %%8$d"
$
$	Format string for vendor name, application name, version, from date, 
$	to date, time stamp.
621  "Deleted Usage Metering license from vendor %%1$s: %%2$s [%%3$s], from %%4$s to %%5$s, timestamp %%6$d"
$
$ NOTRANS.
622  ":"
$
$ The next 3 messages are ADMIN.
623  "No Servers"
624  "Could not verify the server running on node %%1$s"
625  "Could not establish communications with server %%1$s"
$
$ NOTRANS.
626  "?(ls_admin) %%1$s\n"
$
$ ------------------------------ ls_admin XMT Messages -----------------------
$
$ The next 39 messages are ADMIN.
$
627  "LS_ADMIN"
628  "XLs_Admin"
629  "Exit"
630  "Help Index..."
631  "Done"
632  "field"
633  "info"
634  "Add Vendor..."
635  "Add Product..."
636  "Add Licenses..."
637  "Operations:"
638  "Operate On:"
639  "Server"
640  "Vendor"
641  "Product"
642  "License"
643  "Check User File"
644  "Update Server List"
645  "Describe"
646  "Rename..."
647  "Delete..."
648  "Copy Vendor..."
649  "ADD VENDOR"
650  "Vendor Name"
651  "Vendor Id"
652  "Vendor Password"
653  "ADD PRODUCT"
654  "ADD LICENSE"
655  "Product Name"
656  "Product Version"
657  "License Annotation"
658  "License Password"
659  "CONFIRM"
660  "Delete"
661  "Delete Vendor?"
662  "Delete Product?"
663  "COPY VENDOR"
664  "Copy to:"
665  "?(ls_admin)  warning: %%1$i invalid argument(s) following graphic argument delimiter \"%%2$s\" :\n"
$
$ NOTRANS.
666  ">>    %%1$s\n"
$
$ ADMIN.
667  "or  ls_admin %%1$s [ standard X arguments ]\n"
$
$ The next 3 messages are NOTRANS.
668  "s_v_add"
669  "s_p_add"
670  "s_l_add"
$
$ The next 3 messages are ADMIN.
671  "RENAME VENDOR"
672  "New vendor name:"
673  "   RENAME VENDOR\n\nNew vendor name:"
$
$ NOTRANS.
674  "s_v_rename"
$
$ The next 3 messages are ADMIN.
675  "RENAME PRODUCT"
676  "New product name:"
677  "   RENAME PRODUCT\n\nNew product name:"
$
$ The next 3 messages are NOTRANS.
678  "s_p_rename"
679  "s_v_copy"
680  "s_b_t"
$
$ The next 4 messages are ADMIN.
$
681  "Servers"
682  "Vendors"
683  "Products"
684  "Licenses"
$
$ 	--------------------------------- ls_rpt Messages ---------------------------------
$
$ The next 9 messages are NOTRANS.
$
685  "ls_rpt"
$
$	Format string to print logged event information: version, 
$	product id and name, user id, node id, group id,
$	number of licenses, and job id.
$
$	vendor name, product id and name, version, userid, node id, 
$       number of licenses, time and date information, action performed
686  "%%1$-8s %%2$-8s %%3$-8s %%4$-8s %%5$-8s %%6$-3d %%7$-14s %%8$16s\n"
$	vendor name, product id and name, version, userid, node id, 
$       number of licenses, time and date information, action performed
687  "%%1$-8s %%2$-8s %%3$-8s %%4$-8s %%5$-8s %%6$-3d %%7$-14s %%8$s\n"
$
$       Line index position, line index string, vendor name, 
$       product id and name, version, userid, node id, group id, 
688  "%%1$-4d%%2$-6s %%3$-8s %%4$-16s %%5$-8s %%6$-8s %%7$-16s %%8$-8s %%9$-3d %%10$-18s %%11$-38s %%12$s\n"
$
$       This string will give the following in a formated form:
$	"Index","Vendor","Product","Vrsn","User","Node","Group","Amt","Time",
$	"Job Id", "Event"
689  "%%1$-4d%%2$-6s %%3$-8s %%4$-16s %%5$-8s %%6$-8s %%7$-16s %%8$-8s %%9$-3d %%10$-18s %%11$s\n"
$
$       Vendor name, product id and name, version, user id, node id, 
$	number of licenses, time and date information, action performed
690  "%%1$-8s %%2$-8s %%3$-8s %%4$-8s %%5$-8s %%6$-3s %%7$-14s %%8$16s\n"
$
$       Vendor name, product id and name, version, user id, node id, 
$	number of licenses, time and date information, action performed
691  "%%1$-8s %%2$-8s %%3$-8s %%4$-8s %%5$-8s %%6$-3s %%7$-14s %%8$s\n"
$
$       Line index position, line index string, vendor name, 
$	product and name, version, userid, node id, group id, 
$	number of licenses, time and date information, action performed
692  "%%1$-4d%%2$-6s %%3$-8s %%4$-16s %%5$-8s %%6$-8s %%7$-16s %%8$-8s %%9$-3s %%10$-18s %%11$s\n"
693  " "
$
$ The next 3 messages are ADMIN.
694  "-usage"
695  "-help"
696  "-version"
$
$ NOTRANS.
697  "\0"
$
$ The next 13 messages are ADMIN.
$
698  "ab:cdefhjlmn:p:r:st:u:v:x:z"
699  "a"
700  "b"
701  "Bad begin date"
702  "c"
703  "d"
704  "e"
705  "f"
706  "h"
707  "j"
708  "l"
709  "m"
710  "n"
$
$ NOTRANS.
711  "/"
$
$ The next 15 messages are ADMIN.
$
712  "p"
713  "Only ten products may be specified"
714  "r"
715  "1"
716  "2"
717  "You must specify report type '1' or '2'"
718  "s"
719  "t"
720  "Bad end date"
721  "u"
722  "Only ten users may be specified"
723  "v"
724  "Only ten vendors may be specified"
725  "x"
726  "Bad deletion date"
$
$ DEVELOP.
727  "z"
$
$ The next 3 messages are ADMIN.
728  "Extra argument without a matched option in the command line interface.\n"
729  "unknown"
730  "Cannot get local node name\n"
$
$ NOTRANS.
731  ":"
$
$ The next 3 messages are ADMIN.
732  "Error in cleaning log file"
733  "Error in reading log file"
734  "\n\nfinished.\n\n"
$
$ The next 5 messages are NOTRANS.
$
$	Format string for month, day of month, year + 1900
735  "%%1$2d/%%2$2d/%%3$4d"
$
$	Format string for month, day of month, year + 1900
736  "%%1$2d/%%2$2d/%%3$2d"
737  "0"
738  ","
739  "\'"
$
$ The next 2 messages are ADMIN.
740  "could not bind handle: "
741  "iFOR/LS not running on %%1$s"
$
$ The next 7 messages are NOTRANS.
$
742  "?(lsrpt) %%1$s\n"
$
$	Message stating the problem followed by the error text
743  "?(lsrpt) %%1$s %%2$s\n"
744  "%%1$s."
745  "%%1$s"
$
$	Format string for product id and product name
746  "(%%1$ld) %%2$s"
747  "  "
748  " => %%1$s"
$
$	Event, type of license, and computer. ADMIN.
749  "%%1$s%%2$s on mach type %%3$s"
$
$	Event and computer.  ADMIN.
750  "%%1$s on mach type %%2$s"
$
$	Event and time to wait. ADMIN.
751  "%%1$s. wait time = %%2$s"
$
$ NOTRANS.
752  "->%%1$-4d"
$
$ The next 4 messages are ADMIN.
$
$	Event and time that license has been used
753  "%%1$s. time of use = %%2$s"
754  "license timed out. time of use = %%1$s"
755  "license timed out."
756  "up time = %%1$s"
$
$	Format string for "*** svr exited", time and date, 
$       time that server was up for.  NOTRANS.
757  "%%1$-49s%%2$-14s %%3$s\n"
$
$ ADMIN.
758  "*** svr exited"
$
$ The next 2 messages are NOTRANS.
$	Format string for starting line index, starting line string,
$       "*** svr exited", time and date, time that server was up for
759  "%%1$-4d%%2$-6s %%3$-74s%%4$-18s %%5$s\n"
$
$       Format string for "*** svr started", time and date,
760  "%%1$-49s%%2$-14s\n"
$
$ ADMIN.
761  "*** svr started"
$
$	Format string for starting line index, "*** svr started", time and date
$ NOTRANS.
762  "%%1$-10d %%2$-74s%%3$-18s\n"
$
$ The next 10 messages are ADMIN.
$
$	Format string for Vendor name, time and date, vendor message
763  "%%1$-8s *** msg: %%2$s %%3$s\n"
$
$	Format string for line index, " ", vendor name, time and date, 
$	vendor message
764  "%%1$-4d%%2$6s %%3$-8s    *** message: %%4$s %%5$s\n"
$
$	Format string with time/date followed by message
765  "%%1$-14s *** FATAL ERROR *** %%2$s\n"
$
$	Format string with line index, " ", time/date and the message
766  "%%1$-4d%%2$6s %%3$-8s    *** FATAL ERROR *** %%4$s\n"
767  "vendor added"
768  "vendor added to license data base"
769  "vendor deleted"
770  "vendor deleted from license data base"
771  "renamed %%1$s"
772  "vendor renamed to %%1$s"
$
$	Format string for action (to database) and license type. NOTRANS.
773  "%%1$s %%2$s"
$
$	Format string for action (to database), license type, 
$	begin date, and end date. ADMIN.
774  "%%1$s %%2$s (start date: %%3$s, exp date %%4$s)"
$
$ NOTRANS.
775  "        "
$
$ The next 6 messages are ADMIN.
$
776  "product added"
$
$	Begin date and end date 
777  "product added (start date: %%1$s, exp date %%2$s)"
778  "product deleted"
$
$	Begin date and end date
779  "product deleted (start date: %%1$s, exp date %%2$s)"
780  "product renamed to %%1$s"
781  "unexpected input: type = %%1$ld"
$
$	Format string to print version name, product name, version, requestors,
$       granted, and rejection percentage. NOTRANS.
782  "%%1$-12s %%2$-12s %%3$-12s %%4$-12d %%5$-12d %%6$-d%%%%\n"
$
$ ADMIN.
783  "\n\nfinished.\n"
$
$	Format string to print user name, vendor name, product name, version, 
$	requestors, granted, rejections. NOTRANS.
784  "%%1$-12s %%2$-12s %%3$-12s %%4$-12s %%5$-12d %%6$-12d %%7$-d%%%%\n"
$
$ ADMIN.
785  "\nTotal Users : %%1$d\n"
$
$ NOTRANS.
786  "00:00"
$
$ The next 4 messages are ADMIN.
$
787  "%%1$d days, "
788  "%%1$2d:"
$
$       Format string for minutes and seconds
789  "%%1$2d:%%2$2d"
$
$ The next 2 messages are NOTRANS.
$	Format string for month, day of month, year, hour, and minute of hour
790  "%%1$2d/%%2$2d/%%3$2d %%4$2d:%%5$2d"
$
$	Format string for month, day of month, year, hour, minute, and second
791  "%%1$2d/%%2$2d/%%3$2d %%4$2d:%%5$2d:%%6$2d"
$
$ ADMIN.
792  "%%1$d/%%2$d/%%3$d"
$
$ NOTRANS.
793  "\n\n\n"
$
$ The next 2 messages are ADMIN.
794  "N e t w o r k   L i c e n s e   S e r v e r   U s a g e   R e p o r t (%%1$s)\n"
795  "For: %%1$s\n\n"
$
$	Format string to print vendor, product, version, requests, granted, 
$	rejections. NOTRANS.
796  "%%1$-12s %%2$-12s %%3$-12s %%4$-12s %%5$-12s %%6$-12s\n"
$
$ The next 13 messages are ADMIN.
$
797  "Vendor"
798  "Product"
799  "Vrsn"
800  "Num of Req"
801  "Num Granted"
802  "%% Rejections"
803  "------"
804  "-------"
805  "----"
806  "----------"
807  "-----------"
808  "------------"
809  "N e t w o r k   L i c e n s e   S e r v e r   U s e r   U s a g e   R e p o r t (%%1$s)\n"
$
$	Format string for user, vendor, product, version, number of requests, 
$	number granted, and rejections. NOTRANS.
810  "%%1$-12s %%2$-12s %%3$-12s %%4$-12s %%5$-12s %%6$-12s %%7$-12s\n"
$
$ The next 8 messages are ADMIN.
$
811  "User"
812  "N e t w o r k   L i c e n s e   S e r v e r   L o g   R e p o r t  (%%1$s)\n"
813  "Node"
814  "Amt"
815  "Time"
816  "Event"
817  "---"
818  "-----"
$
$       This string will give the following in a formated form:
$	"Index","Vendor","Product","Vrsn","User","Node","Group","Amt","Time",
$	"Job Id", "Event".  NOTRANS.
819  "%%1$-10s %%2$-8s %%3$-16s %%4$-8s %%5$-8s %%6$-16s %%7$-8s %%8$-3s %%9$-18s %%10$-38s %%11$s\n"
$
$ The next 3 messages are ADMIN.
820  "Index"
821  "Group"
822  "Job Id"
$
$       Line index position, line index string, vendor name, 
$       product id and name, version, userid, node id, group id, 
$	number of licenses, time and date information, job id, action performed
$ NOTRANS.
823  "%%1$-10s %%2$-8s %%3$-16s %%4$-8s %%5$-8s %%6$-16s %%7$-8s %%8$-3s %%9$-18s %%10$s\n"
$
$ The next 12 messages are ADMIN.
$
824  "\n(Network_License_Server_Report) -- Display log of iFOR/LS system\n"
825  "usage: ls_rpt [-n nodename] [-acdefhjlms] [-b mm/dd/yy] [-t mm/dd/yy] \n"
826  "                        [-v vnd_list] [-p product_list] [-u user_list] \n"
827  "                        [-r report_type] [-x mm/dd/yy] \n"
828  "LS_RPT Version %%1$s\n"
829  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
830  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
831  "ConcurrentAccess"
832  "NodeLocked"
833  "UseOnce"
834  "Compound"
835  "UsageMetering"
$
$ NOTRANS.
836  ""
$
$ The next 16 messages are ADMIN.
$
837  "G R A N T E D"
838  "R E L E A S E D"
839  "C H E C K E D"
840  "W A I T I N G"
841  "G R A N T / W A I T"
842  "W A I T   R E M O V E D"
843  "M U L T I P L E   G R A N T"
844  "M U L T I P L E   R E L E A S E"
845  "U S E _ O N C E   U S E D"
846  "C O M P O U N D   U S E D"
847  "V E N D O R  A D D E D"
848  "V E N D O R  D E L E T E D"
849  "V E N D O R  R E N A M E D"
850  "P R O D U C T  A D D E D"
851  "P R O D U C T  D E L E T E D"
852  "P R O D U C T  R E N A M E D"
$
$ The next 3 messages are DEVELOP.
853  "entering: %%1$s()\n"
854  "ls_Log_Rpt10"
855  "ls_Log_Rpt"
$
$ NOTRANS.
856  "\0"
$
$ DEVELOP.
857  "ls_Log_Rpt_11"
$
$ NOTRANS.
858  " "
$
$ -------------------------------- ls_stat Messages ---------------------------
$
$ The next 3 messages are ADMIN.
859  "-usage"
860  "-h"
861  "-version"
$
$ NOTRANS.
862  "\0"
$
$ ADMIN.
863  "All"
$
$ The next 3 messages are NOTRANS.
$
864  "#"
865  "?(ls_stat) %%1$s\n"
$
$	Message describing problem and error text
866  "?(ls_stat) %%1$s - %%2$s\n"
$
$ The next 5 messages are ADMIN.
$ 
867  "LS_STAT Version %%1$s\n"
868  "(c) Copyright 1991,1992,1993,1994, Hewlett-Packard Company, All Rights Reserved\n"
869  "(c) Copyright 1991,1992,1993,1994, Gradient Technologies Inc., All Rights Reserved\n"
870  "License Users\n"
871  "Error at server %%1$s"
$
$ The next 2 messages are NOTRANS.
872  "\n%%1$s\n"
$
$	Product name and product version
873  "\n %%1$s [%%2$s]\n"
$
$ The next 2 messages are ADMIN.
874  "\n  On server: %%1$-16s \n"
875  "  Wait queue length: %%1$d\n"
$
$	Format string for "User", "Node", "Group", "Number", "Start Time"
$ NOTRANS.
876  "        %%1$10s %%2$16s %%3$8s %%4$9s %%5$13s\n"
$
$ The next 5 messages are ADMIN.
$
877  "User"
878  "Node"
879  "Group"
880  "Number"
881  "Start Time"
$
$ The next 2 messages are NOTRANS.
$
$	Format string for user, node, group id, number of licenses, and time
882  "        %%1$10.10s %%2$16.16s %%3$8.8s %%4$9d %%5$25s\n"
883  "          %%1$10.10s %%2$16.16s %%3$8.8s %%4$9d %%5$25s\n"
$
$ The next 10 messages are ADMIN.
$
884  "  Licenses in use: %%1$d\n"
885  "\n Total licenses in use: %%1$d\n"
886  " Total number of users: %%1$d\n"
887  "End of License Users\n"
888  "concurrent"
889  "nodelocked"
890  "use once"
891  "compound"
892  "metered"
893  "Installed Licenses\n"
$
$	Format string for "Licenses", "Type", "In Use", "Start Date", 
$	"Expiration Date".  NOTRANS.
894  "            %%1$10s%%2$12s%%3$8s  %%4$-11s  %%5$-17s\n"
$
$ The next 5 messages are ADMIN.
$
895  "Licenses"
896  "Type"
897  "In Use"
898  "Start Date"
899  "Expiration Date"
$
$	Format string for total number of licenses, license types, 
$	licenses in use, start date of license, end date of license. NOTRANS.
900  "            %%1$10d%%2$12s%%3$8d  %%4$11s  %%5$11s"
$
$	Format string for total number of licenses, license types, 
$	"", start date of license, end date of license. NOTRANS.
901  "            %%1$10s%%2$12s%%3$8s  %%4$11s  %%5$11s"
$
$ ADMIN.
902  " expired\n"
$
$ NOTRANS.
903  "\n"
$
$ ADMIN.
904  "  Total active"
$
$ The next 2 messages are NOTRANS.
$	Format string for number of licenses, "            ",
$	number of licenses in use. 
905  "%%1$8d%%2$12s%%3$8d\n"
906  "            "
$
$ ADMIN.
907  "End of Installed Licenses\n"
$
$ The next 2 messages are NOTRANS.
908  "%%1$d"
909  "          "
$
$ ADMIN.
910  "License Usage\n"
$
$ The next 2 messages are NOTRANS.
911  " ["
912  "]"
$
$ ADMIN.
913  "                                   In Use    Total Active    Available\n"
$
$ The next 2 messages are NOTRANS.
914  "%%1$s\n"
$
$	The product, the number of licenses in use, 
$	the total number of licenses, the number of licenses remaining
915  "  %%1$-32s %%2$6d%%3$16d%%4$13d\n"
$
$ The next 23 messages are ADMIN.
$
916  "End of License Usage\n"
917  "Insufficient memory"
918  "Unable to locate any Network License Servers"
919  "Unable to communicate with  any Network License Servers"
920  "Unable to communicate with server %%s"
921  "Could not get address for %%s"
922  "The Network License Server is not running on %%s"
923  "Error at server %%s"
$
$	Product name, version, node name
924  "There is no product %%1$s [%%2$s] on server %%3$s"
$
$	Product name, node name
925  "There is no product %%1$s on server %%2$s"
$
$	Vendor name, node name
926  "There is no vendor %%1$s on server %%2$s"
$
$	Product name, version
927  "There is no product %%1$s [%%2$s]"
928  "There is no product %%s"
929  "There is no vendor %%s"
930  "iatn:v:p:r:u:z"
931  "i"
932  "a"
933  "t"
934  "n"
935  "v"
936  "p"
937  "r"
938  "u"
$
$ DEVELOP.
939  "z"
$
$ The next 11 messages are ADMIN. 
$
940  "\nCommand Line usage:  ls_stat  {-t | -i | -a | -u } [ options ]\n"
941  "\noptions:\n"
942  "    -n <node name>\n"
943  "    -v <vendor name >\n"
944  "    -p <product name>\n"
945  "    -r <version>\n\n"
946  "Insufficient memory"
947  "Specify -a[llusers] and/or -i[nstall] and/or -t[otal] and/or -u[ser]"
948  "Specify a product with a version"
949  "\nDialog interface usage:  ls_stat\n"
950  "?(ls_stat)  error: %%1$i invalid argument(s) preceeding graphic argument delimiter \"%%2$s\" :\n"
$
$ NOTRANS.
951  ">>    %%1$s\n"
$
$ The next 2 messages are ADMIN.
952  "Wait"
953  "All"
$
$ The next 2 messages are NOTRANS.
954  " ["
955  " ]"
$
$ The next 27 messages are ADMIN.
$
956  "Unable to display any Network License Servers"
957  "Unable to display any products"
958  "Displaying all vendors and products for all servers."
959  "Displaying all vendors and products for server:  %%s"
960  "Displaying all servers for all vendors and products."
961  "Displaying all servers for vendor:  %%s"
$
$    	Product name and version
962  "Displaying all servers for product:  %%1$s [%%2$s]"
963  "Insufficient memory"
964  "Jan "
965  "Feb "
966  "Mar "
967  "Apr "
968  "May "
969  "Jun "
970  "Jul "
971  "Aug "
972  "Sep "
973  "Oct "
974  "Nov "
975  "Dec "
976  "Sun "
977  "Mon "
978  "Tue "
979  "Wed "
980  "Thu "
981  "Fri "
982  "Sat "
$
$ The next 8 messages are NOTRANS.
$
$	Month, day, year
983  "%%1$s %%2$d %%3$d"
$
$	Month, day, year
984  "%%1$s%%2$d %%3$d"
985  " %%1$d"
986  "%%1$d"
987  "0%%1$d"
$
$     Name of day, Name of month, date in month, hour, minute, year
988  "%%1$s%%2$s%%3$s %%4$s:%%5$s %%6$d\n"
989  ":"
990  "\0"
$
$ ------------------------------ ls_stat XMT Messages -------------------------
$
$ The next 15 messages are ADMIN.
$
991  "LS_STAT"
992  "XLs_Stat"
993  "Exit"
994  "Help Index..."
995  "field"
996  "User"
997  "User Name"
998  "License\nInformation:"
999  "Installed"
1000  "Usage"
1001  "All Users"
1002  "All Servers (Update)"
1003  "All Products (Update)"
1004  "Wait"
1005  "?(ls_stat)  warning: %%1$i invalid argument(s) following graphic argument delimiter \"%%2$s\" :\n"
$
$ NOTRANS.
1006  ">>    %%1$s\n"
$
$ The next 3 messages are ADMIN.
$
1007  "or  ls_stat %%1$s [ standard X arguments ]\n"
$
$ ------------------------------ Option Usage Messages ------------------------
$
1008  "usage:%%1$s"
1009  "program"
$
$ The next 4 messages are NOTRANS.
$
1010  "\0"
1011  " %%1$s"
1012  "\n"
1013  "      %%1$s"
$
$ ADMIN.
1014  ":No usage data available"
$
$ The next 3 messages are NOTRANS.
$
1015  ">"
1016  "<"
1017  "+"
$
$ The next 2 are ADMIN.
1018  "input_file"
1019  "output_file"
$
$ The next 2 are NOTRANS.
1020  "#"
1021  ":"
$
$ ADMIN.
1022  "string"
$
$ The next 5 messages are NOTRANS.
$
1023  "="
1024  " -"
1025  " -%%1$c"
$
$ ----------------------------- User File Handler -----------------------------
$
1026  "stat of user file (%%1$s) failed with errno %%2$d - anyone can use product"
1027  " "
$
$ %1 is the user file, %2 is the product. ADMIN.
1028  "user file contains error (%%1$s) - anyone can use product: %%2$s"
$
$ NOTRANS.
1029  "r"
$
$ The next 2 messages are ADMIN.
$
$ %1 is the user file, %2 is the product. 
1030  "user file not found (%%1$s) - anyone can use product: %%2$s"
$
$ %1 is the vendor name, %2 is the product
1031  "vendor/product not found in user file (%%1$s, %%2$s) - anyone can use product"
$
$ The next 2 messages are NOTRANS.
1032  "\0"
1033  "0"
$
$ The next 6 messages are ADMIN.
$
1034  "expected VENDOR token in user file - line %%1$d"
1035  "expected vendor name in user file - line %%1$d"
1036  "expected product name or ALL in user file - line %%1$d"
1037  "expected ALLOW/DISALLOW in user file - line %%1$d"
1038  "user file not found: %%1$s"
1039  "expected number following CRAWL"
$
$ The next 3 messages are NOTRANS.
$
1040  "2.0.1 (GRI 1.1.2e) AIX"
$
$ -------------------------------- TraceBack Messages -------------------------
$
1041  "`node_data/system_logs/netls_error_log"
1042  "netls"
$
$ The next 3 messages are ADMIN.
$
$	The name ("netls"), the version is (2.0), and the task 
1043  "%%1$s version %%2$s\n\nFault occurred in %%3$s"
$
$	The name ("netls") and the version (2.0)
1044  "%%1$s version %%2$s"
$
$	The name ("netls"), the version (2.0), and the error message
1045  "%%1$s version %%2$s\n\n%%3$s"
$
$ -------------------------- Communication Utility Messages -------------------
$
$ The next 13 messages are NOTRANS.
$
1046  " "
1047  "-"
1048  "+"
1049  "%%1$d"
1050  "\0"
1051  "\n"
1052  "\0"
1053  "\n"
$
$ ------------------------------ UAI XMT Messages -----------------------------
$
1054  "%%1$s\n"
$
$ Format string for a message string followed by a variable
1055  "%%1$s: %%2$s\n"
1056  "I"
1057  "S"
1058  "#"
$
$ The next 8 messages are ADMIN.
$
1059  "Help"
1060  "HELP"
1061  "Help On"
1062  "Help Index"
1063  "Help Topics"
1064  "No help available"
1065  "Enter new value"
1066  "Edit the text or click 'Cancel' to\n\\return to the previous value.\n\\\n\\When finished, press:\n\\<return> to enter *only* the characters\n\\to the *left* of the insert cursor,\n\\or press <enter> or click on the 'OK'\n\\button to enter the new value completely."
$
$ The next 2 messages are NOTRANS.
1067  "\0"
1068  " "
$
$ The next 8 messages are ADMIN.
$
1069  "Enter new %%1$s"
1070  "double initialization of help index"
1071  "double initialization"
1072  "notify"
1073  "Dismiss"
1074  "unmapped_form"
1075  "NOTIFICATION"
1076  "Prompt Box"
$
$ NOTRANS.
1077  "r"
$
$ The next 5 messages are ADMIN.
$
1078  "bad help file -- won't open"
1079  "bad help file -- bad format"
1080  "nothing following .I line in"
1081  "no topics, only index in help file"
1082  "expected .S topics in help file, found"
$
$ The next 15 messages are NOTRANS.
$
1083  "form"
1084  "frame"
1085  "sep"
1086  "label"
1087  "push_btn"
1088  "radio_btn"
1089  "check_btn"
1090  "cascade_btn"
1091  "option_btn"
1092  "pulldown_menu"
1093  "radio_box"
1094  "selection_dialog"
1095  "prompt_dialog"
1096  "question_dialog"
1097  "dialog"
$
$ ADMIN.
1098  "work"
$ 
$ The next 2 messages are NOTRANS.
1099  "pop_btn"
1100  "text_button"
$
$ ADMIN.
1101  "Why not?"
$
$ The next 2 messages are NOTRANS.
1102  "list"
1103  "scrolled_text"
$
$ ADMIN.
1104  "topic not found in help file"
$
$ 	------------------------- License Server Utilty Messages --------------------------
$
$ The next 5 messages are NOTRANS.
$
1105  "length of buffer not a multiple of 8 in encrypt_key() (size = %%1$d)\n"
1106  "length of buffer not a multiple of 8 in decrypt_key() (size = %%1$d)\n"
1107  "abcd"
1108  "/"
1109  " "
$
$	Year, month, day. ADMIN message.
1110  "%%1$d %%2$d %%3$d"
$
$	Year, month, day. ADMIN message.
1111  "%%1$d/%%2$d/%%3$d"
$
$ The next 14 messages are NOTRANS.
$
$	Text, Date
1112  "%%1$s%%2$s\n"
1113  "%%1$s\n"
$ 
$
$ ------------------------- License Server Version Messages -------------------
$
1114  "2.0.1 GR1.0.7a Apollo Domain"
1115  "2.0.1 (GR1.0.7a) Intergraph"
1116  "2.0.1 (GR1.0.7a) SVR4"
1117  "2.0.1 (GR1.0.7a) SCO_UNIX"
1118  "2.0.1 (GR1.0.7a) HP-UX"
1119  "2.0.1 (GR1.0.7a) DEC-ULTRIX"
1120  "2.0.1 (GR1.0.7a) DEC-VMS"
1121  "2.0.1 (GR1.0.7a) MIPS-RISCOS"
1122  "2.0.1 (GR1.0.7a) SunOS"
1123  "2.0.1 (GR1.0.7a) SGI"
1124  "2.0.1 (GR1.0.7a) NeXT"
1125  "2.0.1 (GR1.0.7a) OSINFO"
$
$ The next 2 are ADMIN.
1126  "FATAL ERROR: ifor_ls_encode called in nls_lib!!!\n"
1127  "FATAL ERROR: ifor_ls_decode called in nls_lib!!!\n"
$
$ Routines that help to Internationalize scripts
$
$ The next 5 messages are NOTRANS.
$
1128  "netls.cat"
1129  "-f"
1130  "-n"
1131  "-m"
1132  "-s"
$
$ The next 4 are ADMIN.
1133  "Need set number and message number to do catalog lookup\n"
1134  "Set number is %%1$d\nMessage number is %%2$d\n"
1135  "\nPlease indicate your choice [y or n]\n"
1136  "Problem with log_file.  Moving to log_file.error and starting with new log_file.\n"
1137  "OSF/1"
1138  "OS/2"
1139  "OS/400"
1140  "MVS"
1141  "Win/NT"
